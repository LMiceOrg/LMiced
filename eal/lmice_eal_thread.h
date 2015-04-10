#ifndef LMICE_EAL_THREAD_H
#define LMICE_EAL_THREAD_H

#include "lmice_eal_common.h"

#if defined(__APPLE__) || defined(__linux__) || USE_POSIX_THREAD==1

    #include "lmice_eal_thread_pthread.h"

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

