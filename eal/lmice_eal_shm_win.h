#ifndef LMICE_EAL_SHM_WIN_H
#define LMICE_EAL_SHM_WIN_H

#include "lmice_eal_common.h"
#include <stdint.h>
typedef HANDLE  shmfd_t;
typedef void*   addr_t;
#ifdef _W64
struct lmice_shm_s
{
    shmfd_t     fd;
    uint32_t    size;
    uint32_t    reserved;
    addr_t      addr;
    char        name[32];
};
#else
struct lmice_shm_s
{
    shmfd_t     fd;
    uint32_t    size;
    addr_t      addr;
    char        name[32];
};
#endif

#endif /** LMICE_EAL_SHM_WIN_H */

