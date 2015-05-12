#ifndef LMICE_EAL_THREAD_WIN_H
#define LMICE_EAL_THREAD_WIN_H

#include "lmice_eal_common.h"

static eal_pid_t forceinline gettid()
{
    return GetCurrentThreadId();
}

eal_pid_t forceinline pthread_self()
{
    return GetCurrentThreadId();
}

int forceinline pthread_getname_np(DWORD tid, char* name, size_t sz)
{
    UNREFERENCED_PARAM(tid);
    UNREFERENCED_PARAM(name);
    UNREFERENCED_PARAM(sz);
    /* No implementation*/
    return -1;
}

int forceinline pthread_setname_np(const char* name)
{
    UNREFERENCED_PARAM(name);
    return 0;
}

eal_pid_t forceinline getpid()
{
    return GetCurrentProcessId();
}

#endif /** LMICE_EAL_THREAD_WIN_H */

