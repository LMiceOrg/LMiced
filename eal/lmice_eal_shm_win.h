#ifndef LMICE_EAL_SHM_WIN_H
#define LMICE_EAL_SHM_WIN_H

#include "lmice_eal_common.h"

struct lmice_shm_s
{
    HANDLE fd;
    int size;
    uint64_t addr;
    wchar_t name[32];
};

#endif /** LMICE_EAL_SHM_WIN_H */

