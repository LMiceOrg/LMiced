#include "timer_system_time.h"

#include <eal/lmice_trace.h>


#define ACTIVE_TIMER_STATE(state) (state).value[0]=LM_TRIGGERED_STATE;
#define ACTIVE_REFERENCED_ACTION(pact) (pact)->info->state.value[(pact)->pos] = LM_TRIGGERED_STATE
#define CHECK_ACTION_STATE(pact) ( *(uint64_t*)&((pact)->info->state) == 0xFFFFFFFFFFFFFFFFULL )
void forceinline timer_due_scheduler(int64_t now, lm_timer_res_t* tlist, lm_timer_res_t* active_list)
{
    size_t pos = 0;
    lm_timer_res_t *res = NULL;
    lm_ref_act_t *act = NULL;
    do
    {
        for(pos = 0; pos < TIMER_LIST_NEXT_POS; ++pos)
        {
            res = tlist + pos;
            if(res == NULL) break;
            if(res->info == NULL)break;
            if( now >= res->info->due)
            {
                /* Step.1 trigger timer */
                ACTIVE_TIMER_STATE(res->info->timer.state);
                eal_event_awake(res->worker->res.efd);

                /* Step.2 check referenced action */
                act = res->alist;
                while(act != NULL)
                {
                    if(act->info == NULL) break;
                    /* update referenced action */
                    ACTIVE_REFERENCED_ACTION(act);

                    if(CHECK_ACTION_STATE(act))
                    {
                        /* trigger referenced action */
                        eal_event_awake(res->worker->res.efd);
                    }
                    act = act->next;
                }

                /* Step.3 update trigger counter */
                res->info->timer.count ++;
                res->info->timer.begin = now;

                /* Step.4 check and move timer to active list */
                /* size = 0 means infinite */
                if(res->info->timer.count < res->info->size
                        || res->info->size == 0)
                {
                    //ret = add_timer_to_tmlist(active_list, tlist+pos);
                    append_timer_to_tmlist(active_list, res);
                }

                /* delete timer from due list*/
                remove_timer_from_tmlist(tlist, res);
            }

        }
        /* check next array */
        tlist = (lm_timer_res_t *)tlist[TIMER_LIST_NEXT_POS].info;
    } while(tlist != NULL);
}

void forceinline timer_scheduler(int64_t now, lm_timer_res_t * tlist)
{
    int64_t pos = 0;
    lm_timer_res_t *res = NULL;
    lm_ref_act_t *act = NULL;
    do
    {
        for(pos = 0; pos < tlist[TIMER_LIST_NEXT_POS].active; ++pos)
        {
            res = tlist + pos;
            if(res == NULL) break;
            if(res->info == NULL)break;
            if( now >= res->info->timer.begin + res->info->period)
            {
                /* Step.1 trigger timer */
                ACTIVE_TIMER_STATE(res->info->timer.state);
                eal_event_awake(res->worker->res.efd);


                //lmice_debug_print("timer 0x%X triggered\n", res->info->inst_id);

                /* Step.2 check referenced action */
                act = res->alist;
                while(act != NULL)
                {
                    if(act->info == NULL) break;
                    /* trigger action */
                    ACTIVE_REFERENCED_ACTION(act);
                    if(CHECK_ACTION_STATE(act))
                    {
                        /* trigger referenced action */
                        eal_event_awake(res->worker->res.efd);
                    }

                    act = act->next;

                }

                /* Step.3 update trigger counter */
                res->info->timer.count ++;
                res->info->timer.begin = now;

                /* Step.4 check and remove on-shot and size <= count timer */
                /* size = 0 means infinite */
                if(res->info->timer.count >= res->info->size
                        && res->info->size > 0)
                {
                    /* move timer list*/

                    remove_timer_from_tmlist(tlist, res);
                    lmice_debug_print("remove timer list [%u], [%d]\n", pos, tlist[127].active);
                }
            }

        }
        /* check next array */
        tlist = (lm_timer_res_t *)tlist[TIMER_LIST_NEXT_POS].info;
    } while(tlist != NULL);

}

static void forceinline time_update(lm_time_param_t*   pm)
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

    int64_t now = 0;
    int64_t tick = 0;
    lm_res_param_t *pm = (lm_res_param_t*)dwUser;

    time_update(&pm->tm_param);

    now = pm->tm_param.pt->system_time;
    tick = pm->tm_param.pt->tick_time;

    timer_due_scheduler(now, pm->timer_duelist, pm->timer_worklist);
    timer_scheduler(now, pm->timer_worklist);

    if(tick >0)
    {
        timer_due_scheduler(tick, pm->ticker_duelist, pm->ticker_worklist);
        timer_scheduler(tick, pm->ticker_worklist);
    }

}

int stop_time_thread(lm_res_param_t *pm)
{
    timeKillEvent(pm->tm_param.wTimerID);
    timeEndPeriod(pm->tm_param.wTimerRes);

    pm->tm_param.wTimerID = 0;
    pm->tm_param.wTimerRes = 0;
    return 0;

}

int create_time_thread(lm_res_param_t *pm)
{

    TIMECAPS tc;

    pm->tm_param.count = 0;

    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        DWORD err = GetLastError();
        lmice_critical_print("Create time thread failed[%u]\n", err);
        return 1;
    }

    pm->tm_param.wTimerRes = min(max(tc.wPeriodMin, MMTIME_RESOLUTION), tc.wPeriodMax);
    pm->tm_param.wTimerDelay = pm->tm_param.wTimerRes;

    timeBeginPeriod(pm->tm_param.wTimerRes);

    pm->tm_param.wTimerID = timeSetEvent(
                pm->tm_param.wTimerDelay,                // delay
                pm->tm_param.wTimerRes,                  // resolution (global variable)
                time_thread_proc,               // callback function
                (DWORD_PTR)pm,                  // user data
                TIME_PERIODIC );                // single timer event
    if(! pm->tm_param.wTimerID)
        return 1;
    else
        return 0;
}

#elif defined(__LINUX__) || defined(__APPLE__)



#define CLOCKID     CLOCK_REALTIME
#define SIG         SIGRTMIN
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
    } while (0)


static void print_siginfo(siginfo_t *si)
{
    timer_t *tidp;
    int or;

    tidp = si->si_value.sival_ptr;

    printf("    sival_ptr = %p; ", si->si_value.sival_ptr);
    printf("    *sival_ptr = 0x%lx\n", (long) *tidp);

    or = timer_getoverrun(*tidp);
    if (or == -1)
        errExit("timer_getoverrun");
    else
        printf("    overrun count = %d\n", or);
}
static void
time_sig_proc(int sig, siginfo_t *si, void *uc)
{
    time_param_t* pm;
    pm = si->si_value.sival_ptr;
    time_update(pm);

    signal(sig, SIG_IGN);
}

int create_time_thread(time_param_t* pm)
{
    //timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    //long long freq_nanosecs;
    //sigset_t mask;
    struct sigaction sa;

    pm->sig = SIG;
    pm->clockid = CLOCKID;

    /* Establish handler for timer signal */
    printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = time_sig_proc;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1)
        errExit("sigaction");

    /* Block timer signal temporarily */
    printf("Blocking signal %d\n", SIG);
    sigemptyset(&pm->mask);
    sigaddset(&pm->mask, SIG);
    if (sigprocmask(SIG_SETMASK, &pm->mask, NULL) == -1)
        errExit("sigprocmask");

    /* Create the timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = pm;
    if (timer_create(pm->clockid, &sev, &pm->timerid) == -1)
        errExit("timer_create");

    printf("timer ID is 0x%lx\n", (long) timerid);
    /* Start the timer */

    //freq_nanosecs = atoll(argv[2]);
    its.it_value.tv_sec = 0;//freq_nanosecs / 1000000000;
    its.it_value.tv_nsec = 1000000;//freq_nanosecs % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if (timer_settime(pm->timerid, 0, &its, NULL) == -1)
        errExit("timer_settime");
}

int stop_time_thread(time_param_t* pm)
{
    int ret=0;
    printf("Unblocking signal %d\n", SIG);
    if (sigprocmask(SIG_UNBLOCK, &pm->mask, NULL) == -1)
        errExit("sigprocmask");

    ret = timer_delete(pm->timerid);
    return ret;
}

#endif
