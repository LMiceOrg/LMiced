#ifndef LMICE_EAL_THREAD_WIN_H
#define LMICE_EAL_THREAD_WIN_H

#include "lmice_eal_common.h"

forceinline eal_pid_t eal_gettid(void)
{
    return GetCurrentThreadId();
}

forceinline eal_pid_t eal_thisthread_id(void)
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

eal_pid_t forceinline getpid(void)
{
    return GetCurrentProcessId();
}

#endif /** LMICE_EAL_THREAD_WIN_H */

