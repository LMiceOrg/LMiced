#ifndef RESOURCE_SHM_INTERNAL_H
#define RESOURCE_SHM_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define lmice_resource_internal_d

struct lmice_shm_array_head_s
{
     uint64_t lock;         /** shm array's lock */
     int32_t size;          /** current usages of array */
     int32_t count;         /** total size of array */
     int64_t scenario;      /** scenario identity */
     uint64_t next_array;   /** next array address */
     char reserved[32];     /** padding */
};

struct lmice_shm_array_s
{
    uint64_t hash;          /** shm name's hash */
    int32_t size;           /** shm size */
    int32_t count;          /** shm count */
    int64_t  fd;            /** file descriptor */
    uint64_t address;       /** shm address */
    char name[32];          /** shm name */
};


struct lmice_shm_object_s
{
    uint64_t type;
    uint64_t owner;
    uint64_t scenario;
};

typedef struct lmice_shm_object_s lmice_shm_object_t;

typedef struct lmice_shm_array_s lmice_shm_array_t;
typedef struct lmice_shm_array_head_s lmice_shm_array_head_t;

int lmice_remove_shm_by_hval(lmice_shm_array_t* shm_array, uint64_t hval);
int lmice_find_shm_by_hval(lmice_shm_array_t *shm_array, uint64_t hval, lmice_shm_array_t **psa);


#ifdef __cplusplus
}
#endif

#endif /** RESOURCE_SHM_INTERNAL_H */

