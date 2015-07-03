#ifndef LMICE_RING_H
#define LMICE_RING_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 权限管理,数据管理,读写操作
 * 写操作带有tick标识
 */

struct lmice_ring_s
{
    uint32_t rwlock;
    uint32_t writing_pos;
    uint32_t reading_pos;
    uint32_t byte_length;   /** 4 bytes aligned length */
    uint32_t data_size;
    uint32_t data_count;
    uint8_t data[8];        /** data of ring */
};
typedef struct lmice_ring_s lmice_ring_t;

enum lmice_ring_e {
    EAL_RING_WRITE = 0,
    EAL_RING_READ = 1,
    EAL_RING_EOK = 0,
    EAL_RING_ENOTFOUND = 2
};

struct lmice_ring_ops;
typedef lmice_ring_t* (*iterator)(struct lmice_ring_ops*);
struct lmice_ring_ops {
    lmice_ring_t* ring;
    iterator begin;
    iterator end;
};
typedef struct lmice_ring_ops lm_ring;



int lmice_ring_create(lmice_ring_t** ring, unsigned int data_size, unsigned int count);
int lmice_ring_destroy(lmice_ring_t* ring);

int eal_ring_acquire_right(lmice_ring_t* ring, int access, uint64_t id);
int eal_ring_release_right(lmice_ring_t* ring, int access, uint64_t id);

int lmice_ring_write(lmice_ring_t ring, void* data, unsigned int size);
int lmice_ring_read(lmice_ring_t ring, void** data, unsigned int* size);


int eal_create_ring(lm_ring** ppring);


#ifdef __cplusplus
}
#endif

#endif /** LMICE_RING_H */
