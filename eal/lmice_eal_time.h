#ifndef LMICE_EAL_TIME_H
#define LMICE_EAL_TIME_H
#include "lmice_eal_common.h"
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h> /* mach_absolute_time */
#if defined(__MACH__)
forceinline static int init_time(uint64_t* factor) {
    mach_timebase_info_data_t time_base;
    mach_timebase_info(&time_base);
    *factor = time_base.numer / time_base.denom;
    return 0;
}

/* the UTC time since 1970-01-01 */
forceinline static int  get_system_time(int64_t* t) {
    uint64_t tick = 0;
    tick = mach_absolute_time();
    *t = tick / 100ULL;
    return 0;
}

#elif defined(__LINUX__) || defined(__MINGW32__)

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

