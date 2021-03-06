#include "timer_system_time.h"

#include <eal/lmice_trace.h>


#define ACTIVE_TIMER_STATE(state) (state).value[0]=LM_TRIGGERED_STATE;
#define ACTIVE_REFERENCED_ACTION(pact) (pact)->info->state.value[(pact)->pos] = LM_TRIGGERED_STATE
#define CHECK_ACTION_STATE(pact) ( *(uint64_t*)&((pact)->info->state) == 0xFFFFFFFFFFFFFFFFULL )



forceinline void delete_timer_by_pos(uint64_t pos, lm_timer_res_t *res, lm_timer_res_t **tlist)
{
    res->active = LM_TIMER_NOTUSE;
    memmove(tlist+pos, tlist+pos+1, (tlist[TIMER_LIST_NEXT_POS]->active - pos)*sizeof(lm_timer_res_t*) );
    tlist[ tlist[TIMER_LIST_NEXT_POS]->active ] = NULL;
    --tlist[TIMER_LIST_NEXT_POS]->active;
}



forceinline void get_tlist_by_remain(int64_t remain, lm_res_param_t* pm, lm_timer_res_t* val, lm_timer_res_t*** tlist)
{
    lm_timer_info_t *info = val->info;
    if(info->type == TIMER_TYPE)
    {

        if(remain < PERIOD_1)
            *tlist = pm->timer_worklist1;
        else if(remain < PERIOD_2)
            *tlist = pm->timer_worklist2;
        else if(remain < PERIOD_3)
            *tlist = pm->timer_worklist3;
        else
            *tlist = pm->timer_worklist4;

    }
    else
    {

        if(remain < PERIOD_1)
            *tlist = pm->ticker_worklist1;
        else if(remain < PERIOD_2)
            *tlist = pm->ticker_worklist2;
        else if(remain < PERIOD_3)
            *tlist = pm->ticker_worklist3;
        else
            *tlist = pm->ticker_worklist4;

    }
}

forceinline void timer_list_schedule(int64_t now, lm_timer_res_t **tlist, lm_res_param_t*  pm)
{
    uint64_t pos = 0;
    int64_t remain = 0;
    lm_timer_res_t *res = NULL;
    lm_ref_act_t *act = NULL;
    lm_timer_res_t **newlist = NULL;
    do
    {
        for(pos = 1; pos <= tlist[TIMER_LIST_NEXT_POS]->active; ++pos)
        {

            res = tlist[pos];

            /*
             lmice_debug_print("pos[%lld] timer.active[%lld]\n", pos, res->active);
            */

            if(res == NULL) break;
            if(res->active != LM_TIMER_RUNNING ||
                    res->info == NULL)
            {
                /* remove from list */
                lmice_debug_print("remove[%ud].[%p] from list[res.active=%lld]\n",res->info->size,
                                  res,
                                  res->active
                                  );
                delete_timer_by_pos(pos, res, tlist);
                --pos;
                continue;
            }

            if(res->info->timer.begin <=0) /* run at once */
                remain = 0;
            else
                remain = res->info->timer.begin + res->info->period - now;

            /*
             lmice_debug_print("check timer[%lld][%p]\n", res->info->period, tlist);
            */

            if( remain <= 0)
            {
                lmice_debug_print("trigger timer[%lld][%p] [%lld]\n",remain, res, res->active);

                res->info->timer.begin = now;
                remain = res->info->period;

                /* Step.1 check referenced action */
                act = res->alist;
                while(act != NULL)
                {
                    if(act->info == NULL) break;
                    /* trigger action */
                    ACTIVE_REFERENCED_ACTION(act);
                    act = act->next;
                }

                /* Step.2 trigger timer */

                ACTIVE_TIMER_STATE(res->info->timer.state);
                eal_event_awake(res->worker->res.efd);

                /* Step.3 update trigger counter */
                res->info->timer.count ++;
                res->info->timer.begin = now;

                /* Step.4 check and remove on-shot and size <= count timer */
                /* size = 0 means infinite */
                if(res->info->size == 0 ||
                        (res->info->timer.count >= res->info->size
                         && res->info->size > 0) )
                {
                    /* remove from list */
                    delete_timer_by_pos(pos, res, tlist);
                    --pos;

                    /* go for-loop */
                    continue;
                }
            }

            /* move timer to proper list by remain*/
            get_tlist_by_remain(remain, pm, res, &newlist);
            if(tlist != newlist)
            {
                /* move it to newlist */

                append_timer_to_tlist(res, newlist);
                memmove(tlist+pos, tlist+pos+1, (tlist[TIMER_LIST_NEXT_POS]->active - pos)*sizeof(lm_timer_res_t*) );
                tlist[ tlist[TIMER_LIST_NEXT_POS]->active ] = NULL;
                --tlist[TIMER_LIST_NEXT_POS]->active;
                --pos;

            }



        } /* end-for */

        /* get next array */
        tlist = (lm_timer_res_t **)tlist[TIMER_LIST_NEXT_POS]->info;
    } while(tlist != NULL);

}

void forceinline due_list_schedule(int64_t now, lm_timer_res_t** tlist, lm_res_param_t*  pm)
{
    uint64_t pos = 0;
    int64_t remain = 0;
    lm_timer_res_t *res = NULL;
    lm_ref_act_t *act = NULL;
    do
    {
        for(pos = 1; pos <= tlist[TIMER_LIST_NEXT_POS]->active; ++pos)
        {
            res = tlist[pos];
            if(res == NULL) break;
            if(res->active != LM_TIMER_RUNNING)
            {
                /* remove from list */
                delete_timer_by_pos(pos, res, tlist);
                --pos;
                continue;
            }

            if(res->info == NULL)break;
            remain = res->info->due  - now;
            if( remain <= 0)
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
                ++res->info->timer.count;
                res->info->timer.begin = now;

                /* Step.4 check and move timer to active list */
                /* size = 0 means infinite */
                if(res->info->timer.count < res->info->size
                        || res->info->size == 0)
                {
                    append_timer_to_res(pm, res);
                }

                /* delete timer from due list*/
                delete_timer_by_pos(pos, res, tlist);
                --pos;

            }/* end-if :remain */

        } /* end-for :pos*/

        /* check next array */
        tlist = (lm_timer_res_t **)tlist[TIMER_LIST_NEXT_POS]->info;
    } while(tlist != NULL);
}

forceinline void update_time_and_tick(lm_time_param_t* pm)
{
    lm_time_t*      pt = pm->pt;

    /** update system time, every 32 milliseconds
        update tick time when the tick_zero_time be set
    */
    if((pm->count & 0x1f) == 0)
    {
        int64_t btime = pt->system_time;
        get_system_time(&pt->system_time);
        if(pt->tick_zero_time != 0)
        {
            pt->tick_time += pt->tick_rate*(pt->system_time - btime);
        }
    }
    else
    {
        pt->system_time += 10000LL * (int64_t)pm->wTimerDelay;

        if(pt->tick_zero_time != 0)
        {
            pt->tick_time += pt->tick_rate * (int64_t)pm->wTimerDelay;
        }
    }

    pm->count ++;
}

forceinline void schedule_timer_and_ticker(lm_res_param_t* pm)
{
    int64_t now = 0;
    int64_t tick = 0;
    lm_timer_res_t **tlist = NULL;

    now = pm->tm_param.pt->system_time;
    tick = pm->tm_param.pt->tick_time;

    /* check due list */
    tlist = pm->timer_duelist;
    due_list_schedule(now, tlist, pm);

    /* always check worklist1 */
    tlist = pm->timer_worklist1;
    timer_list_schedule(now, tlist, pm);

    if( (pm->tm_param.count & 0xf) == 0)
    {
        tlist = pm->timer_worklist2;
        timer_list_schedule(now, tlist, pm);
    }

    if( (pm->tm_param.count & 0x1f) == 0)
    {

        tlist = pm->timer_worklist3;
        timer_list_schedule(now, tlist, pm);
    }

    if( (pm->tm_param.count & 0x7f) == 0)
    {
        tlist = pm->timer_worklist4;
        timer_list_schedule(now, tlist, pm);

    }

    /*
    if( (pm->tm_param.count % 1000) == 0)
    {
        int64_t cnt = pm->timer_worklist1[0]->active +
                pm->timer_worklist2[0]->active +
                pm->timer_worklist3[0]->active +
                pm->timer_worklist4[0]->active;
        lmice_critical_print("total timer %lld, [%lld]\n", cnt, pm->tm_param.count);
    }
    */

    if(tick >0)
    {
        /* check due list */
        tlist = pm->ticker_duelist;
        due_list_schedule(tick, tlist, pm);

        /* always check worklist1 */
        tlist = pm->ticker_worklist1;
        timer_list_schedule(tick, tlist, pm);

        if( (pm->tm_param.count & 0xf) == 0)
        {
            tlist = pm->ticker_worklist2;
            timer_list_schedule(tick, tlist, pm);
        }

        if( (pm->tm_param.count & 0x1f) == 0)
        {
            tlist = pm->ticker_worklist3;
            timer_list_schedule(tick, tlist, pm);
        }

        if( (pm->tm_param.count & 0x7f) == 0)
        {
            tlist = pm->ticker_worklist4;
            timer_list_schedule(tick, tlist, pm);
        }
    }
}


#if defined(_WIN32)

#if defined(USE_MMTIMER)
/* 1-millisecond target resolution */
#define MMTIME_RESOLUTION 1

void CALLBACK time_thread_proc(UINT wTimerID, UINT msg,
                               DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
    UNREFERENCED_PARAM(wTimerID);
    UNREFERENCED_PARAM(msg);
    UNREFERENCED_PARAM(dw1);
    UNREFERENCED_PARAM(dw2);

    lm_res_param_t *pm = (lm_res_param_t*)dwUser;

    /* update time and tick */
    update_time_and_tick(&pm->tm_param);

    /* resource task proc */
    resource_task_proc(pm);

    /* schedule timer and ticker */
    schedule_timer_and_ticker(pm);

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

#else /* Dont use MMTIMER */

static void time_thread_proc(void* dwUser)
{

    lm_res_param_t *pm = (lm_res_param_t*)dwUser;


    /* update time and tick */
    update_time_and_tick(&pm->tm_param);

    /* resource task proc */
    resource_task_proc(pm);

    /* schedule timer and ticker */
    schedule_timer_and_ticker(pm);


}

int create_time_thread(lm_res_param_t* pm)
{
    lm_timer_ctx_t *ctx = NULL;

    lm_time_param_t*tp =&( pm->tm_param );
    tp->wTimerDelay = 20000llu;
    tp->wTimerRes = 10000llu;
    tp->quit_flag = 0;

    eal_timer_malloc_context(ctx);

    ctx->context = pm;
    ctx->handler = time_thread_proc;
    ctx->interval = tp->wTimerDelay;
    ctx->quit_flag = &(tp->quit_flag);

    return eal_timer_create2(&tp->timer, ctx);

}

int stop_time_thread(lm_res_param_t *pm)
{
    lm_time_param_t*tp =&( pm->tm_param );
    eal_timer_destroy2(tp);
    return 0;
}


#endif /* USE_MMTIMER */

#elif defined(__APPLE__)

#include <sys/time.h>


static void time_thread_proc(void* dwUser)
{

    lm_res_param_t *pm = (lm_res_param_t*)dwUser;


    /* update time and tick */
    update_time_and_tick(&pm->tm_param);

    /* resource task proc */
    resource_task_proc(pm);

    /* schedule timer and ticker */
    schedule_timer_and_ticker(pm);


}

int create_time_thread(lm_res_param_t* pm)
{
    lm_timer_ctx_t *ctx = NULL;

    lm_time_param_t*tp =&( pm->tm_param );
    tp->wTimerDelay = 10000llu;
    tp->wTimerRes = 10000llu;
    tp->quit_flag = 0;

    eal_timer_malloc_context(ctx);

    ctx->context = pm;
    ctx->handler = time_thread_proc;
    ctx->interval = tp->wTimerDelay;
    ctx->quit_flag = &(tp->quit_flag);

    return eal_timer_create2(&tp->timer, ctx);

}

int stop_time_thread(lm_res_param_t *pm)
{
    lm_time_param_t*tp =&( pm->tm_param );
    eal_timer_destroy2(tp);
    /*
    tp->quit_flag = 1;
    pthread_join(tp->timer, NULL);
    */
    /*
     * dispatch_release(tp->wTimerID);
    */
    return 0;
}


#elif defined(__LINUX__)



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
