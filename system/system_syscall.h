#ifndef SYSTEM_SYSCALL_H
#define SYSTEM_SYSCALL_H

#include "eal/lmice_eal_hash.h"
#include "lmice_eal_common.h"
#include "lmice_eal_shm.h"
#include "lmice_eal_spinlock.h"
#include "lmice_trace.h"
#include "resource/resource_manage.h"

#include <string.h>

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

/* delete message resource */
forceinline int lmsys_delete_message(lm_mesg_res_t* res) {
    int ret = 0;
    ret = eal_shm_close(res->sfd, res->addr);
    return ret;
}

/* create subscribe message in shared memory */
forceinline int
lmsys_create_subscribe_message(lm_mesg_res_t *res, uint64_t id, uint32_t size) {
    int ret = 0;
    lmice_shm_t     shm;
    eal_shm_zero(&shm);
    shm.size = size;
    shm.name[0] = 'S';
    eal_shm_hash_name(id, shm.name+1);
    ret = eal_shm_create_or_open(&shm);
    if(ret != 0) {
        lmice_error_print("lmsys_create_subscribe_message call eal_shm_create_or_open(%s, %d) failed[%d]\n", shm.name, shm.size, ret);
        return ret;
    }

    res->addr = shm.addr;
    res->sfd = shm.fd;
    return ret;
}

/* create publish message in shared memory */
forceinline int lmsys_create_publish_message(lm_mesg_res_t *res, uint64_t id, uint32_t size) {
    int ret = 0;
    lmice_shm_t     shm;
    eal_shm_zero(&shm);
    shm.size = size;
    shm.name[0] = 'P';
    eal_shm_hash_name(id, shm.name+1);
    ret = eal_shm_create(&shm);
    if(ret != 0) {
        lmice_error_print("lmsys_create_publish_message call eal_shm_create(%s, %d) failed[%d]\n", shm.name, shm.size, ret);
        return ret;
    }

    res->addr = shm.addr;
    res->sfd = shm.fd;
    return ret;
}

/* register publish message */
forceinline int
lmsys_register_publish(lm_res_list_t*rr_list, lm_worker_res_t* worker, uint64_t type_id, uint64_t inst_id, uint32_t size)
{
    int ret = 0;
    size_t i=0;
    lm_worker_t * wk = NULL;
    lm_mesg_info_t *info = NULL;
    lm_mesg_res_t *res = NULL;

    /* acquire resource publish right */
    ret = lmres_acquire_publish_right(rr_list, type_id, inst_id, worker->info->inst_id);
    if(ret != 0)
        return ret;

    wk = (lm_worker_t *)worker->res.addr;
    eal_spin_lock(&wk->board.lock);

    /* for(i=0; i<128; ++i ) {
        info = wk->mesg +i;
    */
    for(i=0; i<wk->res_capacity; ++i) {
        info = &wk->res[i].message;
        res = worker->mesg + i;
        if(info->inst_id == 0) {
            ret = lmsys_create_publish_message(res, inst_id, size);
            if(ret != 0) {
                break;
            }
            info->type = PUBLISH_RESOURCE_TYPE;
            info->size = size;
            info->period = 0;
            info->inst_id = inst_id;
            info->type_id = type_id;
            break;
        }
    }

    eal_spin_unlock(&wk->board.lock);
    return ret;

}

/* register subscribe object */
forceinline int
lmsys_register_subscribe(lm_res_list_t*rr_list, lm_worker_res_t* worker, uint64_t type_id, uint64_t inst_id, uint32_t size, lm_mesg_res_t *res) {
    lm_worker_t * wk = NULL;
    lm_mesg_info_t *info = NULL;
    size_t i = 0;
    int ret = 0;

    if(inst_id == 0)
        return 1;


    /* acquire resource subscribe right */
    ret = lmres_acquire_subscribe_right(rr_list, type_id, inst_id, worker->info->inst_id);
    if(ret != 0)
        return ret;


    wk = (lm_worker_t *)worker->res.addr;
    ret = eal_spin_trylock(&wk->board.lock);
    if(ret != 0)
        return ret;

    if(size == 0)
        size = DEFAULT_SHM_SIZE;

    for(i=0; i<wk->res_capacity; ++i) {
        info = &wk->res[i].message;
    /*for(i=0; i<128; ++i ) {
        info = wk->mesg +i; */
        res = worker->mesg + i;
        if(info->inst_id == 0) {
            ret = lmsys_create_subscribe_message(res, inst_id, size);
            if(ret != 0) {
                break;
            }
            info->type = SUBSCRIBE_RESOURCE_TYPE;
            info->size = size;
            info->period = 0;
            info->inst_id = inst_id;
            info->type_id = type_id;

            break;
        }
    }

    eal_spin_unlock(&wk->board.lock);
    return ret;

}

forceinline int
lmsys_register_subscribe_type(lm_resset_list_t*rr_list, lm_worker_res_t* worker, uint64_t type_id,  uint32_t size) {
    lm_worker_t * wk = NULL;
    lm_mesg_info_t *info = NULL;
    lm_mesg_res_t *res = NULL;
    size_t i = 0;
    int ret = 0;



    /* acquire resource set subscribe right */
    ret = lmresset_acquire_subscribe_right(rr_list, type_id, worker->info->inst_id);
    if(ret != 0)
        return ret;


    wk = (lm_worker_t *)worker->res.addr;
    ret = eal_spin_trylock(&wk->board.lock);
    if(ret != 0)
        return ret;

    if(size == 0)
        size = DEFAULT_SHM_SIZE;

    for(i=0; i<wk->res_capacity; ++i) {
        info = &wk->res[i].message;
    /*for(i=0; i<128; ++i ) {
        info = wk->mesg +i;*/
        res = worker->mesg + i;
        if(info->inst_id == 0) {
            ret = lmsys_create_subscribe_message(res, type_id, size);
            if(ret != 0) {
                break;
            }
            info->type = SUBSCRIBE_RESOURCESET_TYPE;
            info->size = size;
            info->period = 0;
            info->inst_id = 0;
            info->type_id = type_id;

            break;
        }
    }

    eal_spin_unlock(&wk->board.lock);
    return ret;
}

/** register timer event */
forceinline int
lmsys_register_timer(lm_worker_res_t* worker, eal_pid_t process_id, int period, int size, int due, uint64_t* event_id) {

    lm_timer_info_t *info = NULL;
    lm_worker_t * wk = NULL;
    size_t i = 0;
    int ret = TIMER_TYPE;
    uint64_t inst_id;

    inst_id = eal_hash64_fnv1a(&process_id, sizeof(process_id));
    inst_id = eal_hash64_more_fnv1a(&ret, sizeof(ret), inst_id);
    inst_id = eal_hash64_more_fnv1a(&due, sizeof(due), inst_id);
    inst_id = eal_hash64_more_fnv1a(&period, sizeof(period), inst_id);
    inst_id = eal_hash64_more_fnv1a(&size, sizeof(size), inst_id);

    lmice_critical_print("lmsys_register_timer call register_timer\n");
    wk = (lm_worker_t *)worker->res.addr;
    ret = eal_spin_trylock(&wk->board.lock);
    if(ret != 0)
        return ret;

    ret = 1;
    for(i=0; i<wk->res_capacity; ++i) {
        info = &wk->res[i].timer;
    /*for(i=0; i<128; ++i ) {
        info = wk->timer +i;*/
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

    eal_spin_unlock(&wk->board.lock);

    return ret;
}


forceinline int
lmsys_register_timer_callback(lmflt_list_t* list, uint64_t timer_id, void* pdata, lmice_event_callback cb, uint64_t* id) {
    int ret;
    lmflt_cond_t cond;

    memset(&cond, 0, sizeof(lmflt_cond_t));
    cond.tid.object_id = timer_id;
    ret = lmflt_append_task_filter(list, LMICE_TASK_PROC_OUT, LMICE_TASK_PROC_TERMINATE, LMICE_TASK_PROC_LEVEL3, &cond, pdata, cb, id);
    return ret;
}

forceinline int
lmsys_register_recv_event(lm_res_param_t* pm, int fd, uint64_t recv_id) {
    eal_aio_t *aio = NULL;
    int ret = 1;

    /* create new io event */
    eal_spin_lock(&pm->new_aio_lock);
    if(pm->new_aio_size < DEFAULT_LIST_SIZE) {
        aio = pm->new_aio_task + pm->new_aio_size;
        ++pm->new_aio_size;
        eal_aio_add_read(aio, fd, recv_id);
        ret = 0;
    }
    eal_spin_unlock(&pm->new_aio_lock);

    return ret;
}

forceinline int
lmsys_register_recv_callback(lmflt_list_t* list, uint64_t type_id, uint64_t obj_id, void* pdata, lmice_event_callback cb, uint64_t *id) {
    int ret = 1;
    lmflt_cond_t cond;
    int plevel = 0;

    /* Filter condition */
    memset(&cond, 0, sizeof(lmflt_cond_t));
    cond.tid.object_id = obj_id;
    cond.tid.type_id = type_id;
    if(obj_id == 0)
        plevel = LMICE_TASK_PROC_LEVEL1;
    else
        plevel = LMICE_TASK_PROC_LEVEL3;

    ret = lmflt_append_task_filter(list, LMICE_TASK_PROC_OUT, LMICE_TASK_PROC_PASS, plevel, &cond, pdata, cb, id);
    return ret;
}
#endif /** SYSTEM_SYSCALL_H */
