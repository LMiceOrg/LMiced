#ifndef LMICE_EAL_SHM_WIN_H
#define LMICE_EAL_SHM_WIN_H

#include "lmice_eal_common.h"
#include <stdint.h>

#ifdef _W64
struct lmice_shm_s
{
    HANDLE fd;
    uint32_t size;
    uint32_t reserved;
    uint64_t addr;
    char name[32];
};
#else
struct lmice_shm_s
{
    HANDLE fd;
    int32_t size;
    uint64_t addr;
    char name[32];
};
#endif

#endif /** LMICE_EAL_SHM_WIN_H */

