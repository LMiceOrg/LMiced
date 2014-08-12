#ifndef LMICE_RING_H
#define LMICE_RING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct lmice_ring_s* lmice_ring_t;

int lmice_ring_create(lmice_ring_t* ring, int size, int count);
int lmice_ring_destroy(lmice_ring_t ring);

int lmice_ring_write(lmice_ring_t ring, void* data, int size);
int lmice_ring_read(lmice_ring_t ring, void** data, int* size);

#ifdef __cplusplus
}
#endif

#endif /** LMICE_RING_H */
