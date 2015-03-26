#ifndef LMICE_EAL_TIME_H
#define LMICE_EAL_TIME_H

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#include <time.h>

/*POSIX.1-2001*/
int forceinline get_system_time(int64_t* t)
{
    int ret = 0;
    struct timespec tp;
    ret = clock_gettime(CLOCK_REALTIME, &tp);
    if(ret == 0)
    {
        *t = (int64_t)tp.tv_sec*10000000LL + tp.tv_nsec/100LL;
    }
    return ret;
}

#elif defined(_WIN32)
#include "lmice_eal_time_win.h"
#else
#error("No time implementation!")
#endif

#endif /** LMICE_EAL_TIME_H */

