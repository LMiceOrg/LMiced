#ifndef TIMER_SYSTEM_TIME_H
#define TIMER_SYSTEM_TIME_H

#include <eal/lmice_eal_common.h>
#include <eal/lmice_eal_time.h>

#include <stdint.h>


struct lmice_time_s
{
    int64_t system_time;
    /* 10000 --> 1:1,  100 --> 0.01, 1e6 --> 100:1 */
    int64_t tick_rate;
    int64_t tick_time;
    int64_t tick_zero_time;
};
typedef struct lmice_time_s lm_time_t;

#ifdef _WIN32
struct time_param_s
{
    lm_time_t*  pt;
    MMRESULT    wTimerID;
    UINT        wTimerRes;
    UINT        wTimerDelay;
    uint64_t    count;

};
#elif defined(__LINUX__) || defined(__APPLE__)

#include <unistd.h>
#include <signal.h>
#include <time.h>

struct time_param_s
{
    timer_t     timerid;
    sigset_t    mask;
    int         sig;
    int         clockid;
    lm_time_t*  pt;
    uint64_t    count;
};

#else
#error(No time_param implementation!)
#endif
typedef struct time_param_s time_param_t;

static void forceinline time_update(time_param_t*   pm)
{
    lm_time_t*      pt = pm->pt;

    /** update system time, every 32 times
        update tick time when the tick_zero_time be set
    */
    if((pm->count % 32) == 0)
    {
        int64_t btime = pt->system_time;
        get_system_time(&pt->system_time);
        pt->tick_time += pt->tick_rate*(pt->system_time - btime);
    }
    else
    {
        pt->system_time += 10000LL*pm->wTimerDelay;

        if(pt->tick_zero_time != 0)
        {
            pt->tick_time += pt->tick_rate * pm->wTimerDelay;
        }
    }

    pm->count ++;
}

int create_time_thread(time_param_t* pm);
int stop_time_thread(time_param_t* pm);

#endif /** TIMER_SYSTEM_TIME_H */

