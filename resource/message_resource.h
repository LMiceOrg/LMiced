#ifndef MESSAGE_RESOURCE_H
#define MESSAGE_RESOURCE_H

#include <stdint.h>

#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_atomic.h"

#define LMICE_MSG_BLOB_AVAILABLE 0
#define LMICE_MSG_BLOB_SHARED_USING 1
#define LMICE_MSG_BLOB_UNIQUE_USING 2

/**
 * @brief The lmice_message_blob_s struct
 * 消息块
 */
struct lmice_message_blob_s {
    volatile int64_t lock;  /* sync purpose 0: available, 1: using */
    uint64_t reference_count; /* many subscriber process the same one */
    char data[8];
};
typedef struct lmice_message_blob_s lmmsg_blob_t;

/**
 * @brief The lmice_message_s struct
 * 消息
 */
struct lmice_message_s {
    uint16_t qsize;         /* queue size */
    uint16_t bsize;         /* blob size(bytes) 16+data's length */
    uint32_t size;          /* message memory size */
    lmmsg_blob_t blob;
};
typedef struct lmice_message_s lmmsg_t;

/* for initialize purpose */
forceinline void lmmsg_init_by_size(lmmsg_t* msg, uint32_t size, uint16_t data_size) {
    memset(msg, 0, size);
    msg->size = size;
    if(data_size % 8 == 0) {
        msg->bsize = data_size+16;
    } else {
        msg->bsize = data_size + 24 - (data_size % 8);
    }
    msg->qsize = size/msg->bsize;
}

/* for write purpose */
forceinline int lmmsg_acquire_unique_blob(lmmsg_t* msg, lmmsg_blob_t* blob) {
    int ret = 1;
    size_t i=0;
    int64_t val;
    blob = &msg->blob;
    for(i=0; i<msg->qsize; ++i) {
        val = eal_compare_and_swap64(&blob->lock, LMICE_MSG_BLOB_AVAILABLE, LMICE_MSG_BLOB_UNIQUE_USING);
        if(val == LMICE_MSG_BLOB_AVAILABLE) {
            /* success got a blob, reset ref_count */
            blob->reference_count = 0;
            ret = 0;
            break;
        }
        blob =(lmmsg_blob_t*)( (int64_t*)blob + msg->bsize / 8 );
    }
    return ret;
}

#define lmmsg_release_unique_blob(blob) (blob)->lock = LMICE_MSG_BLOB_AVAILABLE

#endif /** MESSAGE_RESOURCE_H */
