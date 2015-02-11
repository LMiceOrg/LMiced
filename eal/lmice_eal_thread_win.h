#ifndef LMICE_EAL_THREAD_WIN_H
#define LMICE_EAL_THREAD_WIN_H

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

DWORD forceinline pthread_self()
{
    return GetCurrentThreadId();
}

#endif /** LMICE_EAL_THREAD_WIN_H */

