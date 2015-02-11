#ifndef RESOURCE_META_DATA_INTERNAL_H
#define RESOURCE_META_DATA_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define lmice_resource_meta_data_d

#define META_DATA_MAX_SIZE  128
struct lmice_meta_data_s
{
    uint64_t hval_type;             /** message's type hash */
    int32_t version;                /** message's version */
    int32_t count;                  /** meta data parameters' count */
    char data[META_DATA_MAX_SIZE];  /** meta data */
};

struct lmice_meta_data_array_s
{
    uint64_t lock;
    int32_t count;
    int32_t size;
    uint64_t next_array;
    char reserved[META_DATA_MAX_SIZE - 8];
};

typedef struct lmice_meta_data_s lmice_meta_data_t;
typedef struct lmice_meta_data_array_s lmice_meta_data_array_t;

#ifdef __cplusplus
}
#endif


#endif /** RESOURCE_META_DATA_INTERNAL_H */
