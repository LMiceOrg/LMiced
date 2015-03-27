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
struct lmice_time_parameter_s
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

struct lmice_time_parameter_s
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
typedef struct lmice_time_parameter_s lm_time_param_t;

int create_time_thread(lm_time_param_t* pm);
int stop_time_thread(lm_time_param_t* pm);

#endif /** TIMER_SYSTEM_TIME_H */

