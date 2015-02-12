#ifndef LMICE_EAL_THREAD_H
#define LMICE_EAL_THREAD_H

#if defined(__APPLE__) || defined(__linux__)
    #include "lmice_eal_thread_pthread.h"
#elif defined(_WIN32)
    #include "lmice_eal_thread_win.h"
#else
    #error(Unsupported thread library)
#endif

#endif /** LMICE_EAL_THREAD_H */

