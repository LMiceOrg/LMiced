#ifndef SYSTEM_SYSCALL_H
#define SYSTEM_SYSCALL_H

#include "lmice_eal_common.h"
#include "lmice_eal_shm.h"
#include "lmice_eal_spinlock.h"
#include "lmice_trace.h"
#include "resource/resource_manage.h"

/** a message has only one publish right, and one or many subscribe rights
 *
 * 1. worker create publish, when
 * not exist, create and get publish right
 * already exist and publish right, failed to create
 * already exist and subscribe right, get publish right
 * 2. worker create subscribe, when
 * not exist, create and get subscribe right
 * already exist and publish right, get subscribe right
 * already exist and subscribe right, get subscribe right
*/

/* create message resource */
forceinline int lmsys_create_message(lm_mesg_res_t *res, uint64_t inst_id, uint32_t size)
{
    int ret = 0;
    //    lmice_event_t   evt;
    lmice_shm_t     shm;
    eal_shm_zero(&shm);
    shm.size = size;
    eal_shm_hash_name(id, shm.name);
    ret = eal_shm_create(&shm);
    if(ret != 0)
    {
        lmice_critical_print("create shm[%s] size(%d) failed[%d]\n", shm.name, shm.size, ret);
        return ret;
    }

    res->addr = shm.addr;
    res->sfd = shm.fd;

}

/* register publish message */
forceinline int
lmsys_register_publish(lm_worker_t* worker, uint64_t type_id, uint64_t inst_id, uint32_t size)
{
    int ret = 0;
    size_t i=0;
    lm_mesg_info_t *info = NULL;
    lmice_shm_t     shm;

    eal_shm_zero(&shm);
    shm.size = size;
    eal_shm_hash_name(inst_id, shm.name);

    /* check exist */
    ret = eal_shm_open(&shm, O_RDWR);
    if(ret == 0) {
        /* check exist's right */
    }
    for(i=0; i< 128; ++i)
    {
        info = worker->mesg + i;
        if(info->inst_id == 0)
        {
            info->type = PUBLISH_RESOURCE_TYPE;
            info->size = size;
            info->period = 0;
            info->inst_id = inst_id;
            info->type_id = type_id;
        }
    }


}

/** register timer event */
forceinline int
lmsys_register_timer(lm_worker_res_t* worker, uint64_t session_id, eal_pid_t process_id, int period, int size, int due, uint64_t* event_id) {

    lm_timer_info_t *info = NULL;
    lm_worker_t * wk = NULL;
    size_t i = 0;
    int ret = TIMER_TYPE;
    uint64_t inst_id;

    inst_id = eal_hash64_fnv1a(&session_id, sizeof(session_id));
    inst_id = eal_hash64_more_fnv1a(&process_id, sizeof(process_id), inst_id);
    inst_id = eal_hash64_more_fnv1a(&ret, sizeof(ret), inst_id);
    inst_id = eal_hash64_more_fnv1a(&due, sizeof(due), inst_id);
    inst_id = eal_hash64_more_fnv1a(&period, sizeof(period), inst_id);
    inst_id = eal_hash64_more_fnv1a(&size, sizeof(size), inst_id);

    lmice_critical_print("lmsys_register_timer call register_timer\n");
    wk = (lm_worker_t *)worker->res.addr;
    ret = eal_spin_trylock(&wk->lock);
    if(ret != 0)
        return ret;

    ret = 1;
    for(i=0; i<128; ++i ) {
        info = wk->timer +i;
        if(info->inst_id == 0) {
            ret = 0;
            info->type = TIMER_TYPE;
            info->size = size;
            info->period = period;
            info->due = due;
            info->inst_id = inst_id;
            info->timer.count = 0;
            info->timer.begin = 0;
            *event_id = inst_id;
            break;
        }
    }

    eal_spin_unlock(&wk->lock);

    return ret;
}

forceinline int
lmsys_register_callback()
#endif /** SYSTEM_SYSCALL_H */
