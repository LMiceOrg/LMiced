#ifndef LMICE_EAL_THREAD_H
#define LMICE_EAL_THREAD_H

#include "lmice_eal_common.h"

#include <stdint.h>

#if defined(_WIN32)
typedef uint32_t eal_tid_t;
typedef int32_t  eal_pid_t;
#else
typedef uint64_t eal_tid_t;
typedef int32_t  eal_pid_t;
#endif

#if defined(__LINUX__) || defined(USE_POSIX_THREAD)

#include "lmice_eal_thread_pthread.h"

#elif defined(__APPLE__)

    #include <sys/syscall.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <stdint.h>

forceinline eal_tid_t eal_gettid()
{
    return (uintptr_t)(void*)pthread_self();
}


#elif defined(_WIN32) && defined(_MSC_VER) /** MSC */
    #include "lmice_eal_thread_win.h"
    #define Thread __declspec( thread )

#elif defined(__MINGW32__)
#include "lmice_eal_thread_pthread.h"

static forceinline  int pthread_getname_np(DWORD tid, char* name, size_t sz)
{
    UNREFERENCED_PARAM(tid);
    UNREFERENCED_PARAM(name);
    UNREFERENCED_PARAM(sz);
    /* No implementation*/
    return -1;
}

forceinline static int pthread_setname_np(const char* name)
{
    UNREFERENCED_PARAM(name);
    return 0;
}

#else
    #error(Unsupported thread library)
#endif

#endif /** LMICE_EAL_THREAD_H */

