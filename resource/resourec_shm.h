#ifndef RESOUREC_SHM_H
#define RESOUREC_SHM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmice_eal_shm.h"

#if !defined(lmice_resource_internal_d)
struct lmice_shm_array_s;
typedef struct lmice_shm_array_s lmice_shm_array_t;

struct lmice_shm_object_s;
typedef struct lmice_shm_object_s lmice_shm_object_t;
#endif

int lmice_create_shm_array(lmice_shm_array_t **shm_array);
int lmice_destory_shm_array(lmice_shm_array_t* shm_array);
int lmice_terminate_shm_array(lmice_shm_array_t* shm_array);

int lmice_append_shm(lmice_shm_array_t* shm_array, lmice_shm_t* shm);
int lmice_remove_shm(lmice_shm_array_t* shm_array, lmice_shm_t* shm);

#ifdef __cplusplus
}
#endif

#endif /** RESOUREC_SHM_H */

