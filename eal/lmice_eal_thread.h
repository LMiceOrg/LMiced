#ifndef LMICE_EAL_THREAD_H
#define LMICE_EAL_THREAD_H

#if !defined(PTHREAD)
    #include "lmice_eal_thread_win.h"
#else
    #include "lmice_eal_thread_pthread.h"
#endif

#endif /** LMICE_EAL_THREAD_H */

