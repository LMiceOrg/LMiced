#include "resource_shm_internal.h"
#include "resourec_shm.h"

#include "lmice_eal_align.h"
#include "lmice_eal_atomic.h"
#include "lmice_eal_hash.h"
#include "lmice_eal_spinlock.h"
#include "lmice_eal_malloc.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define SHM_ARRAY_SIZE          1024
#define HASH_CHALLENGE_BEGIN    "SharedMemory//Resource//LMice"
#define HASH_CHALLENGE_END      "hehao@tsinghua.org.cn"
#define LOCK_LOOP_COUNT         20000000LL

static inline __attribute__((always_inline))
uint64_t lock_shm_array(lmice_shm_array_t* shm_array)
{
    lmice_shm_array_head_t *head = (lmice_shm_array_head_t *)shm_array;
    return eal_spin_trylock(&head->lock);

}

static inline __attribute__((always_inline))
uint64_t unlock_shm_array(lmice_shm_array_t* shm_array)
{
    lmice_shm_array_head_t *head = (lmice_shm_array_head_t *)shm_array;
    return eal_spin_unlock(&head->lock);
}

static inline __attribute__((always_inline))
uint64_t shm_hash(lmice_shm_t* shm)
{
    uint64_t hval = eal_hash64_fnv1a( HASH_CHALLENGE_BEGIN, sizeof(HASH_CHALLENGE_BEGIN) -1 );
    hval = eal_hash64_more_fnv1a(shm->name, sizeof(shm->name), hval);
    hval = eal_hash64_more_fnv1a( HASH_CHALLENGE_END, sizeof(HASH_CHALLENGE_END) -1, hval);
    return hval;
}

static inline __attribute__((always_inline))
void hash_to_name(uint64_t hval, char* name)
{
    const char* hex_list="0123456789ABCDEF";
    for(int i=0; i<8; ++i)
    {
        name[i*2] = hex_list[ *( (uint8_t*)&hval+i) >> 4];
        name[i*2+1] = hex_list[ *( (uint8_t*)&hval+i) & 0xf ];
    }
}

int lmice_create_shm_array(lmice_shm_array_t** shm_array)
{
    EAL_STRUCT_ALIGN(lmice_shm_array_t);
    EAL_STRUCT_ALIGN(lmice_shm_array_head_t);
    COMPILE_TIME_ASSERT(sizeof(lmice_shm_array_t) == sizeof(lmice_shm_array_head_t));

    lmice_shm_array_head_t * head = (lmice_shm_array_head_t *) eal_malloc ( sizeof(lmice_shm_array_head_t) * SHM_ARRAY_SIZE );
    if(head == NULL)
            return ENOMEM;

    memset(head, 0, sizeof(lmice_shm_array_head_t) * SHM_ARRAY_SIZE );
    head->count = SHM_ARRAY_SIZE;
    *shm_array = (lmice_shm_array_t*) head;
    return 0;
}

int lmice_destory_shm_array(lmice_shm_array_t* shm_array)
{
    lmice_shm_array_head_t *head = (lmice_shm_array_head_t *)shm_array;
    uint64_t next_array = head->next_array;
    uint64_t locked;

    /* lock the array */
    locked = lock_shm_array(shm_array);
    if(locked != 0)
        return ETIMEDOUT;

    /* destory next shm */
    while(next_array != 0)
    {
        head = (lmice_shm_array_head_t*)next_array;
        next_array = head->next_array;
        eal_free( head );
    }

    eal_free(shm_array);


    return 0;

}


int lmice_append_shm(lmice_shm_array_t *shm_array, lmice_shm_t *shm)
{
    int ret;
    uint64_t locked;
    lmice_shm_t lshm;
    lmice_shm_array_head_t* head;
    lmice_shm_array_t* sa;

    eal_shm_zero(&lshm);
    lshm.size = shm->size;
    uint64_t hval = shm_hash(shm);
    hash_to_name(hval, lshm.name);

    ret = eal_shm_create(&lshm);
    if(ret != 0)
        return ret;

    /* lock shm_array */
    locked = lock_shm_array(shm_array);
    if(locked != 0)
        return ETIMEDOUT;

    head = (lmice_shm_array_head_t*)shm_array;

    while(1) {
        if( head->count < SHM_ARRAY_SIZE -1) /** the first array is available */
        {
            head->count ++;
            sa = (lmice_shm_array_t*)(head + head->size);
            sa->hash = hval;
            sa->address = lshm.addr;
            sa->size = lshm.size;
            sa->fd = lshm.fd;
            break;
        }
        else if(head->next_array == 0 ) /** the next array is not available */
        {
            ret = lmice_create_shm_array(&sa);
            if(ret != 0)
                return ret;
            head->next_array = (uint64_t)sa;
            head = (lmice_shm_array_head_t*)head->next_array;
        }
        else                            /** the next array is available */
        {
            head = (lmice_shm_array_head_t*)head->next_array;
        }

    }

    /* unlock shm_array */
    locked = unlock_shm_array(shm_array);
    return locked;
}

int lmice_remove_shm(lmice_shm_array_t *shm_array, lmice_shm_t *shm)
{
    uint64_t hval = shm_hash(shm);
    return lmice_remove_shm_by_hval(shm_array, hval);
}


int lmice_terminate_shm_array(lmice_shm_array_t *shm_array)
{
    lmice_shm_array_head_t *head = (lmice_shm_array_head_t *)shm_array;
    uint64_t next_array = head->next_array;

    /* destory next shm */
    while(next_array != 0)
    {
        head = (lmice_shm_array_head_t*)next_array;
        next_array = head->next_array;
        eal_free( head );
    }

    eal_free(shm_array);


    return 0;
}

/** resource_shm_internal */

int lmice_remove_shm_by_hval(lmice_shm_array_t *shm_array, uint64_t hval)
{
    int ret = 1;
    int removed = 0;
    uint64_t locked;
    lmice_shm_t lshm;
    lmice_shm_array_head_t* head;
    lmice_shm_array_t* sa;

    eal_shm_zero(&lshm);
    hash_to_name(hval, lshm.name);

    /* lock shm_array */
    locked = lock_shm_array(shm_array);
    if(locked != 0)
        return ETIMEDOUT;

    head = (lmice_shm_array_head_t*)shm_array;

    while(head)
    {
        int32_t i;
        for(i=1; i<= head->count; ++i)
        {
            sa = (lmice_shm_array_t*)(head + i);
            if(sa->hash == hval)
            {
                /* shm destory */
                lshm.addr = sa->address;
                lshm.fd = sa->fd;
                lshm.size = sa->size;
                ret = eal_shm_destroy(&lshm);

                /* memmove shm_array and update head size */
                memmove(sa, sa+1, sizeof(lmice_shm_array_t)* (head->count -i) );
                head->count --;

                /* set removed mark, to break the while loop */
                removed = 1;
                break;
            }
        }

        if(removed == 1)
            break;

        head = (lmice_shm_array_head_t*)head->next_array;
    }

    /* unlock shm_array */
    locked = unlock_shm_array(shm_array);
    return 0;
}

int lmice_find_shm_by_hval(lmice_shm_array_t* shm_array, uint64_t hval, lmice_shm_array_t** psa)
{
    int ret = 1;
    int founded = 0;
    uint64_t locked;
    lmice_shm_array_head_t* head;
    lmice_shm_array_t *sa;


    /* lock shm_array */
    locked = lock_shm_array(shm_array);
    if(locked != 0)
        return ETIMEDOUT;

    head = (lmice_shm_array_head_t*)shm_array;

    while(head)
    {
        int32_t i;
        for(i=1; i<= head->count; ++i)
        {
            sa = (lmice_shm_array_t*)(head + i);
            if(sa->hash == hval)
            {
                *psa = sa;
                /* set founded mark, to break the while loop */
                founded = 1;
                ret = 0;
                break;
            }
        }

        if(founded == 1)
            break;

        head = (lmice_shm_array_head_t*)head->next_array;
    }

    /* unlock shm_array */
    locked = unlock_shm_array(shm_array);
    return ret;
}
