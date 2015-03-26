#include "timer_system_time.h"

#include <eal/lmice_trace.h>


#if defined(_WIN32)

/* 1-millisecond target resolution */
#define MMTIME_RESOLUTION 1

void CALLBACK time_thread_proc(UINT wTimerID, UINT msg,
                               DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    UNREFERENCED_PARAM(wTimerID);
    UNREFERENCED_PARAM(msg);
    UNREFERENCED_PARAM(dw1);
    UNREFERENCED_PARAM(dw2);

    time_param_t*   pm = (time_param_t*)dwUser;
    time_update(pm);
}

int stop_time_thread(time_param_t* pm)
{
    timeKillEvent(pm->wTimerID);
    timeEndPeriod(pm->wTimerRes);

    pm->wTimerID = 0;
    pm->wTimerRes = 0;
    return 0;

}

int create_time_thread(time_param_t* pm)
{

    TIMECAPS tc;

    pm->count = 0;

    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        DWORD err = GetLastError();
        lmice_critical_print("Create time thread failed[%u]\n", err);
        return 1;
    }

    pm->wTimerRes = min(max(tc.wPeriodMin, MMTIME_RESOLUTION), tc.wPeriodMax);
    pm->wTimerDelay = pm->wTimerRes;

    timeBeginPeriod(pm->wTimerRes);

    pm->wTimerID = timeSetEvent(
                pm->wTimerDelay,                      // delay
                pm->wTimerRes,                      // resolution (global variable)
                time_thread_proc,                // callback function
                (DWORD_PTR)pm,                      // user data
                TIME_PERIODIC );                // single timer event
    if(! pm->wTimerID)
        return 1;
    else
        return 0;
}

#elif defined(__LINUX__) || defined(__APPLE__)
#endif
