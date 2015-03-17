
#include "eal/lmice_eal_common.h"
#include <eal/lmice_eal_shm.h>
#include <eal/lmice_eal_spinlock.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

/** 有言者自为名 */
#define MAX_RESOURCE_COUNT 256
uint64_t lm_res_lock = 0;
lmice_shm_t lm_res_list[MAX_RESOURCE_COUNT];

//创建资源
int lmice_create_resource(uint64_t id, int size, void** ptr)
{
    int ret;
    size_t i;
    lmice_shm_t* shm;

    ret = eal_spin_trylock(&lm_res_lock);
    if(ret != 0)
        return ret;
    for(i = 0; i< sizeof(lm_res_list)/sizeof(lmice_shm_t); ++i)
    {
        shm = &lm_res_list[i];
        if(shm->fd == 0)
        {
            shm->fd = INVALID_HANDLE_VALUE;
            break;
        }

    }
    eal_spin_unlock(&lm_res_lock);

    if(shm->fd != INVALID_HANDLE_VALUE)
        return ERANGE;

    eal_shm_zero(shm);
    eal_shm_hash_name(id, shm->name);
    shm->size = size;

    ret = eal_shm_create(shm);
    if(ret != 0)
        return ret;



    *ptr = reinterpret_cast<void*>(shm->addr);
    return 0;
}

int lmice_destroy_resource(uint64_t id)
{
    int ret;
    size_t i;
    lmice_shm_t sh;
    lmice_shm_t *shm = &sh;

    eal_shm_zero(shm);
    eal_shm_hash_name(id, shm->name);

    ret = eal_spin_trylock(&lm_res_lock);
    if(ret != 0)
        return ret;

    for(i = 0; i< sizeof(lm_res_list)/sizeof(lmice_shm_t); ++i) {
        shm = &lm_res_list[i];
        if(shm->fd != 0) {
            if(memcmp(shm->name, sh.name, 7+16) == 0) {
                eal_shm_destroy(shm);
                break;
            }
        }
    }

    eal_spin_unlock(&lm_res_lock);

    return 0;
}

int lmice_open_resource(uint64_t id, void** ptr, int32_t* size = 0)
{
    int ret;
    size_t i;
    lmice_shm_t sh;
    lmice_shm_t *shm = &sh;

    eal_shm_zero(shm);
    eal_shm_hash_name(id, shm->name);

    ret = eal_spin_trylock(&lm_res_lock);
    if(ret != 0)
        return ret;

    for(i = 0; i< sizeof(lm_res_list)/sizeof(lmice_shm_t); ++i) {
        shm = &lm_res_list[i];
        if(shm->fd != 0) {
            if(memcmp(shm->name, sh.name, 7+16) == 0) {
                *ptr = reinterpret_cast<void*>(shm->addr);
                if(size != 0)
                    *size = shm->size;
                break;
            }
        }
    }

    eal_spin_unlock(&lm_res_lock);

    return 0;
}


int lmice_register_publish(uint64_t id, int32_t size)
{

}
