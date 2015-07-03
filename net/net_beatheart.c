#include "net_beatheart.h"

#include "lmice_eal_endian.h"
#include "lmice_eal_spinlock.h"
#include "lmice_core.h"

#include "system/system_syscall.h"
#include <string.h>

struct lmice_ring_head_s
{
    volatile uint32_t lock;
    uint16_t size;
    uint16_t capacity;
    void*   next;
};
typedef struct lmice_ring_head_s lm_ring_hd;

#define LMNET_BEATHEART_PKGLIST_INIT(rhd) \
    rhd->size = 1;  \
    rhd->capacity = LMNET_BEATHEART_LIST_SIZE;  \
    rhd->next = NULL;

#define LMNET_BEATHEART_CONTENT_INIT(ctn) \
    ctn->size = sizeof(lmnet_bh_ctn_t); \
    ctn->worker_size = DEFAULT_WORKER_SIZE; \
    ctn->lcore_size = 8;    \
    ctn->memory_size = 32;  \
    ctn->net_bankwidth = 1000;  \
    ctn->worker_usage = 0;  \
    ctn->lcore_usage = 0;   \
    ctn->memory_usage = 0;  \
    ctn->net_usage = 0;

/** initialize beatheart service
 * 1. init socket
 * create socket handle
 * 2. init data
 * init server package list
 * init client package
*/

/* initialize beatheart resource */
int lmnet_beatheart_init(lmnet_bpkg_t *pkg)
{
    int ret = 0;
    lmnet_bmsg_t *msg = &(pkg->msg);
    lmnet_bctn_t *ctn = &(msg->ctn);

    /* init package header */
    pkg->endian = eal_is_little_endian();
    pkg->padding = LMICE_PADDING_DEFAULT;
    pkg->headlen = sizeof(lmnet_bpkg_t) - sizeof(lmnet_bmsg_t);
    pkg->version = LMICE_VERSION;
    memcpy(pkg->meta_data, LMNET_BEATHEART_METADATA, sizeof(LMNET_BEATHEART_METADATA) -1);

    /* init message header */
    msg->sys_type = lmice_net_beatheart_type();
    msg->evt_tick = 0;
    msg->obj_inst = lmice_net_beatheart_inst(ctn->net_cfg, ctn->net_addr);

    /* init content */
    ctn->size = sizeof(lmnet_bctn_t);
    ctn->worker_size = 200;
    ctn->lcore_size = 8;
    ctn->memory_size = 32;
    ctn->net_bankwidth = 1000;
    ctn->worker_usage = 0;
    ctn->lcore_usage = 0;
    ctn->memory_usage = 0;
    ctn->net_usage = 0;

    return ret;

}


/** server mode beartheart routine
 * process incoming beatheart package
 * mantain inter-node state
 * dispatch received data to IO event-poll
*/

int lmnet_beatheart_recv(void* task, void* pdata) {
    size_t i = 0;
    lmnet_bpkg_t* bh_pkg = NULL;
    eal_aio_data_t *data = (eal_aio_data_t *)pdata;
    lm_res_param_t *res_param = (lm_res_param_t *)data->pdata;
    lmnet_bpkg_t* new_pkg = (lmnet_bpkg_t*)data->buff;
    /* find the position */
    for(i = 0; i< DEFAULT_LIST_SIZE; ++i) {
        bh_pkg = res_param->bh_list + i;
        if(bh_pkg->msg.obj_inst == new_pkg->msg.obj_inst) {
            /* update beatheart package */
            memcpy(bh_pkg, new_pkg, sizeof(lmnet_bpkg_t));
            return 0;
        } else if(bh_pkg->msg.obj_inst == 0) {
            /* meet an empty package, so fill it here */
            memcpy(bh_pkg, new_pkg, sizeof(lmnet_bpkg_t));
            return 0;
        }
    }

    lmice_debug_print("receive a new beatheart\n");
}


/** client mode beartheart routine
 * register timer to finish gather task
 * gather host loan state
 * timely send [this] node beatheart package
*/

int lmnet_beatheart_send(void* task, void* pdata) {
    lm_res_param_t *res_param = (lm_res_param_t *)pdata;
    lmnet_bpkg_t* bh_pkg = res_param->bh_list;
    struct addrinfo* remote = res_param->bh_param.remote;
    lmnet_bctn_t* ctn = &bh_pkg->msg.ctn;
    eal_socket_t sock = res_param->bh_param.sock_client;
    int sz = 0;
    int ret = 0;

    (void)task;

    /* update beatheart state */
    eal_core_get_properties(&ctn->lcore_size, &ctn->memory_size, &ctn->net_bankwidth);
    /* send beatheart package */
    sz=sendto(sock, bh_pkg, sizeof(lmnet_bpkg_t), 0, remote->ai_addr, remote->ai_addrlen);
    if(sz == -1) {
        ret = errno;
        lmice_error_print("lmnet_beatheart_send call sendto failed[%d]\n", ret, remote->ai_addrlen);
    } else {
        lmice_debug_print("lmnet_beatheart_send successfully finished\n");
    }

    return ret;
}

/* create beatheart service */
int lmnet_beatheart_create(lm_res_param_t *res_param)
{
    int ret = 0;
    uint64_t timer_id;
    lm_worker_res_t *worker = res_param->res_worker;

//    lmnet_bprm_t* param = (lmnet_bprm_t*)malloc(sizeof(lmnet_bprm_t));
    eal_inc_param *pm = &(res_param->bh_param);
    /* sock create */
    eal_inc_create_client(pm);
    eal_inc_create_server(pm);

    pm->type_id = lmice_net_beatheart_type();
    pm->inst_id = lmice_net_beatheart_inst(0, pm->local_addr);

    /* register publish */
    lmsys_register_publish(&res_param->res_right, worker, pm->type_id, pm->inst_id, sizeof(lmnet_bpkg_t));
    /* register timer-event 30 seconds */
    lmsys_register_timer(worker, getpid(), 300000000, 0, 0, &timer_id);
    /* register timer-callback for send beatheart (act as client) */
    lmsys_register_timer_callback(res_param->taskfilter_list, timer_id, res_param, lmnet_beatheart_send, &res_param->bh_send_flt);

    /* register subscribe */
    lmsys_register_subscribe_type(&res_param->resset_right, worker, pm->type_id, 0);
    /* register io-callback for receive beatheart (act as server) */
    lmsys_register_recv_event(res_param, pm->sock_server, pm->type_id);
    /* register package filter for receive beatheart (act as task filter) */
    lmsys_register_recv_callback(res_param->taskfilter_list, pm->type_id,
                                 ANY_INSTANCE_ID, res_param, lmnet_beatheart_recv, &res_param->bh_revc_flt);

    /* notify worker */
    return ret;
}

/* destroy beatheart service */
int lmnet_beatheart_destroy(lm_res_param_t *pm)
{
    int ret = 0;
//    lmnet_bprm_t* param = (lmnet_bprm_t*)malloc(sizeof(lmnet_bprm_t));;
//    lmnet_beatheart_client_delete(param);
//    lmnet_beatheart_server_delete(param);
//    lmnet_beatheart_final(param);

    return ret;
}
