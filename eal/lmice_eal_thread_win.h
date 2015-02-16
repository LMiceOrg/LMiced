#ifndef LMICE_EAL_THREAD_WIN_H
#define LMICE_EAL_THREAD_WIN_H

#include "lmice_eal_common.h"

DWORD forceinline pthread_self()
{
    return GetCurrentThreadId();
}

int forceinline pthread_getname_np(DWORD tid, char* name, size_t sz)
{
    /* No implementation*/
    return -1;
}

int forceinline pthread_setname_np(const char* name)
{
    return 0;
}

DWORD forceinline getpid()
{
    return GetCurrentProcessId();
}

#endif /** LMICE_EAL_THREAD_WIN_H */

