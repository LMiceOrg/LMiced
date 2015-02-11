#include "lmice_ring.h"

#include <stdlib.h>

int lmice_ring_create(lmice_ring_t *ring, int size, int count)
{
    size_t sz = size*count+sizeof(struct lmice_ring_s) - 8;
    if( (sz % 4) != 0)
        sz += 4 - (sz%4);
    *ring = (lmice_ring_t)malloc( sz );

    (*ring)->rwlock = 0;
/**    __sync_val_compare_and_swap(&(*ring)->rwlock, 0, 1); */

    return 0;
}

int lmice_ring_destroy(lmice_ring_t ring)
{
    free(ring);

    return 0;
}
