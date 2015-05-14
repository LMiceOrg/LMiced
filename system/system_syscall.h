#ifndef SYSTEM_SYSCALL_H
#define SYSTEM_SYSCALL_H

#include "lmice_eal_common.h"
#include "lmice_eal_shm.h"
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

#endif /** SYSTEM_SYSCALL_H */
