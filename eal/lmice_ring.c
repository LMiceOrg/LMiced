#include "lmice_ring.h"

#include <stdlib.h>
#include <errno.h>

int lmice_ring_create(lmice_ring_t **ring, unsigned int data_size, unsigned int count)
{
    size_t sz = data_size*count+sizeof(struct lmice_ring_s) - 8;
    if( (sz % 4) != 0)
        sz += 4 - (sz%4);
    *ring = (lmice_ring_t*)malloc( sz );

/**    __sync_val_compare_and_swap(&(*ring)->rwlock, 0, 1); */

    return 0;
}

int lmice_ring_destroy(lmice_ring_t *ring)
{
    free(ring);

    return 0;
}

static lm_ring_data_t* begin(lm_ring* pr) {
    return pr->ring->ctx;
}

int eal_create_ring(lm_ring** ppring) {
    lm_ring* pr = *ppring;
    pr = (lm_ring*)malloc(sizeof(lm_ring));
    pr->begin = begin;
    return 0;
}

/**
 * @brief eal_ring_create 从内存上创建环形容器
 * @param mem_address 内存地址
 * @param mem_size  内存长度(bytes)
 * @param length    数据块队列长度
 * @param size      数据块长度(bytes)
 * @param ppring    返回环形容器指针
 * @return
 */
int eal_ring_create(void* mem_address, uint32_t mem_size, uint32_t queue_length, uint32_t block_size, lmice_ring_t** ppring) {
    int ret = 0;
    lmice_ring_t* ring = 0;

    if(bytes_length < CTX_POS)
        return EEAL_RING_OUTOFMEMORY;

    *ppring = (lmice_ring_t*)mem_address;
    ring = *ppring;

    /* Init ring structure */
    memset(ring, 0, CTX_POS);

    /*  memory count */
    ring->mmcount = 1;

    /*  mmblock */
    ring->mmblock[0].address = (lm_ring_data_t*)((uintptr_t)address + CTX_POS);
    ring->mmblock[0].size = mem_size - CTX_POS;
    ring->mmblock[0].count = (mem_size - CTX_POS) / block_size;

    /*  block count */
    ring->blkcount = ring->mmblock[0].count;
    ring->blksize = block_size;
    /*  capacity size */
    ring->capacity = ring->mmblock[0].count;

    /*  queue length */
    if(queue_length > ring->capacity) {
        ring->length = ring->capacity;
        ret = EEAL_RING_OUTOFMEMORY;
    } else {
        ring->length = queue_length;
    }



    return ret;

}
