#ifndef LMICE_EAL_THREAD_WIN_H
#define LMICE_EAL_THREAD_WIN_H

#include "lmice_eal_common.h"

DWORD forceinline pthread_self()
{
    return GetCurrentThreadId();
}

#endif /** LMICE_EAL_THREAD_WIN_H */

