#ifndef LMICE_EAL_TIME_H
#define LMICE_EAL_TIME_H

#if defined(__APPLE__) || defined(__linux__) || defined(__MINGW32__)
#include "lmice_eal_common.h"
#include <unistd.h>
#include <time.h>
#include <stdint.h>

/*POSIX.1-2001*/
forceinline static int  get_system_time(int64_t* t)
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

#elif defined(_WIN32) && !defined(__MINGW32__)
#include "lmice_eal_time_win.h"
#else
#error("No time implementation!")
#endif

#endif /** LMICE_EAL_TIME_H */

