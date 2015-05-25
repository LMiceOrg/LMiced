#include "net_beatheart.h"

#include "lmice_eal_endian.h"
#include "lmice_eal_spinlock.h"

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
int lmnet_beatheart_init(lmnet_bprm_t* param)
{
    int ret = 0;
    lmnet_bpkg_t *pkg = &(param->bh_packge);
    lmnet_bmsg_t *msg = &(pkg->msg);
    lmnet_bctn_t *ctn = &(msg->ctn);
    lmnet_bpkg_t *pkglist = param->bh_pkg_list;
    lm_ring_hd* rhd = (lm_ring_hd*)(void*)pkglist;

    /* create mc socket handle, add membership */
    ret = eal_wsa_create_mc_handle(&(param->net_param));

    /* init inter-node beatheart list */
    LMNET_BEATHEART_PKGLIST_INIT(rhd);

    /* register publish beatheart */
    ret = eal_spin_lock(&param->worker->lock);

    eal_spin_unlock(&param->worker->lock);
    /* init client package */
    memset(pkg, 0, sizeof(lmnet_bpkg_t));

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

/* finalize beatheart resource */
int lmnet_beatheart_final(lmnet_bprm_t* bh_param)
{
    /* stop mc socket handle */
    return 0;
}

/** server mode beartheart routine
 * process incoming beatheart package
 * mantain inter-node state
 * dispatch received data to IO event-poll
*/

/* create server role */
int lmnet_beatheart_server_create(lmnet_bprm_t* param)
{
    int ret = 0;
    HANDLE hdl = NULL;
    eal_wsa_service_param* pm = &(param->net_param);

    /* bind handle to event-io */
    hdl = CreateIoCompletionPort((HANDLE)(pm->hd->nfd),
                                 pm->cp,
                                 (ULONG_PTR)pm->hd,
                                 0);
    if(hdl == NULL) {
        lmice_error_print("CreateIoCompletionPort failed\n");
        closesocket(pm->hd->nfd);
        eal_wsa_remove_handle(pm->hd);
        return -1;
    }

    /* acquire data from io list */
    eal_iocp_append_data(pm->ilist, pm->hd->inst_id, &pm->data);
    pm->data->quit_flag = 0;

    /* start recv routine */
    WSARecvFrom(pm->hd->nfd,
                &(pm->data->data),
                1,
                &(pm->data->recv_bytes),
                &(pm->data->flags),
                (struct sockaddr*)&(pm->hd->addr),
                &(pm->hd->addrlen),
                &(pm->data->overlapped),
                NULL);
    return ret;
}

/* delete server role */
int lmnet_beatheart_server_delete(lmnet_bprm_t* param)
{
    int ret = 0;
    eal_wsa_service_param* pm = &(param->net_param);

    /* notice server to quit */
    pm->data->quit_flag = 1;

    return ret;
}


/** client mode beartheart routine
 * register timer to finish gather task
 * gather host loan state
 * timely send [this] node beatheart package
*/

/* create beatheart client role */
int lmnet_beatheart_client_create(lmnet_bprm_t* bh_param)
{
    int ret = 0;
    lmnet_bpkg_t *pkg = &(bh_param->bh_packge);
    lmnet_bmsg_t *msg = &(pkg->msg);
    lmnet_bctn_t *ctn = &(msg->ctn);

    return ret;
}

/* delete beatheart client role */
int lmnet_beatheart_client_delete(lmnet_bprm_t* bh_param)
{
    int ret = 0;

    return ret;
}

/* create beatheart service */
int lmnet_beatheart_create(lm_res_param_t *res_param)
{
    int ret = 0;
    lm_worker_res_t *worker = res_param->res_worker[0];

    lmice_critical_print("sizeof lmnet_bprm_t %u\n", sizeof(lmnet_bprm_t));
//    lmnet_bprm_t* param = (lmnet_bprm_t*)malloc(sizeof(lmnet_bprm_t));
    eal_inc_param *pm = &(res_param->bh_param);
    /* sock create */
    eal_inc_create_client(pm);
    eal_inc_create_server(pm);
    /* register timer-event 30 seconds */
    lmsys_register_timer(worker, 0, getpid(), 300000000, 0, 0, &pm->id);
    /* register timer-callback for send beatheart */


//    lmnet_beatheart_server_create(param);
//    lmnet_beatheart_client_create(param);
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
