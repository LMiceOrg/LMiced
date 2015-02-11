#include "resource_meta_data_internal.h"
#include "resource_meta_data.h"

#include "lmice_eal_spinlock.h"
#include "lmice_eal_malloc.h"

#include <errno.h>

#define MAX_META_DATA_SIZE 1024

int lmice_create_meta_array(     lmice_meta_data_t   **meta_data)
{
    lmice_meta_data_array_t * array = (lmice_meta_data_array_t *)eal_malloc(sizeof(lmice_meta_data_array_t)* MAX_META_DATA_SIZE );
    if(array == NULL)
        return ENOMEM;

    memset(array, 0, sizeof(lmice_meta_data_array_t));
    array->count = MAX_META_DATA_SIZE;
    array->size = 0;

    *meta_data = (lmice_meta_data_t*)array;
    return 0;
}

int lmice_destory_meta_array(    lmice_meta_data_t   *meta_data)
{
    lmice_meta_data_array_t *head = (lmice_meta_data_array_t *)meta_data;
    uint64_t next_array = head->next_array;
    uint64_t locked;

    /* Try lock the array */
    locked = eal_spin_trylock(&head->lock);
    if(locked != 0)
        return ETIMEDOUT;

    /* destory next shm */
    while(next_array != 0)
    {
        head = (lmice_meta_data_array_t*)next_array;
        next_array = head->next_array;
        eal_free( head );
    }

    eal_free(meta_data);

    return 0;
}

int lmice_terminate_meta_array(lmice_meta_data_t   *meta_array)
{
    lmice_meta_data_array_t *head = (lmice_meta_data_array_t *)meta_array;
    uint64_t next_array = head->next_array;

    /* destory next shm */
    while(next_array != 0)
    {
        head = (lmice_meta_data_array_t*)next_array;
        next_array = head->next_array;
        eal_free( head );
    }

    eal_free(meta_array);

    return 0;
}

int lmice_append_meta_data(lmice_meta_data_array_t* meta_array, lmice_meta_data_t* meta_data)
{
    int ret;
    uint64_t locked;
    lmice_meta_data_t *md;
    lmice_meta_data_array_t *head, *newarray;

    locked = eal_spin_trylock(&meta_array->lock);
    if(locked != 0)
        return ENOLCK;

    /* Create new meta_data */
    head = meta_array;

    while(1)
    {
        if(head->count < MAX_META_DATA_SIZE -1)
        {
            md = (lmice_meta_data_t*)head+head->count;
            memcpy(md, meta_data, sizeof(lmice_meta_data_t));
            head->count ++;
            break;
        }
        else if(head->next_array == 0)
        {
            ret = lmice_create_meta_array(&newarray);
            if(ret != 0)
                return ret;
            head->next_array = (uint64_t)newarray;
            head = newarray;

        }
        else
        {
            head = (lmice_meta_data_array_t*) head->next_array;
        }
    }
    return 0;
}

int lmice_remove_meta_data(lmice_meta_data_array_t* meta_array, lmice_meta_data_t* meta_data)
{

}
