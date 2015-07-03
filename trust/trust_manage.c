#include "rtspace.h"

#include "trust_manage.h"

#include "eal/lmice_eal_spinlock.h"
#include <eal/lmice_trace.h>

#include <signal.h>
#include <errno.h>

/* 500-millisecond trust interval */
#define TRUST_PERIOD_MSEC 500
/* 50-millisecond trust resolution */
#define TRUST_RESOLUTION_MSEC 50

#if defined(_WIN32)




forceinline void trust_resource_compute(lm_trust_t* pt) {
    size_t pos = 0;
    int ret = 0;
    lm_worker_info_t *inst=NULL;
    lm_server_t *server = pt->server;
    ret = eal_spin_lock(&server->board.lock);
    for(inst = &server->worker, pos = 0; pos < DEFAULT_WORKER_SIZE; ++inst, ++pos) {
        HANDLE hProcess = NULL;
        HANDLE hThread = NULL;
        DWORD err;

        /* if it's empty, go check next */
        if( inst->inst_id == 0)
            continue;

        /* check version, only check LMICE_VERSION */
        if(inst->version != LMICE_VERSION)
            continue;

        /* check process state */
        if(inst->process_id != 0) {
            hProcess = OpenProcess(PROCESS_SET_INFORMATION|PROCESS_TERMINATE,
                                   FALSE,
                                   inst->process_id);
            if(hProcess == NULL) {
                err = GetLastError();
                lmice_debug_print("process[%ud] open failed[%ud]\n", inst->process_id, err);
                memset(inst, 0, sizeof(lm_worker_info_t));
                inst->state = WORKER_DEAD;
                eal_event_awake(pt->efd);
                continue;
            }
            CloseHandle(hProcess);
        }

        /* check thread state */
        if(inst->thread_id != 0) {
            hThread = OpenThread(THREAD_SET_INFORMATION|THREAD_TERMINATE,
                                 FALSE,
                                 inst->thread_id);
            if(hThread == NULL){
                err = GetLastError();
                lmice_debug_print("thread[%ud] open failed[%lld]\n", inst->thread_id, err);
                memset(inst, 0, sizeof(lm_worker_info_t));
                inst->state = WORKER_DEAD;
                eal_event_awake(pt->efd);
                if(hProcess)
                    CloseHandle(hProcess);
                continue;
            }
            CloseHandle(hThread);
        }

        if(inst->process_id == 0 && inst->thread_id == 0){
            memset(inst, 0, sizeof(lm_worker_info_t));
        }

    }
    eal_spin_unlock(&server->board.lock);
}


int create_trust_thread(lm_trust_t* pt)
{
    lm_timer_ctx_t *ctx = NULL;
    pt->wTimerDelay = TRUST_PERIOD_MSEC * 10000ull;
    pt->wTimerRes = TRUST_RESOLUTION_MSEC * 10000ull;

    eal_timer_malloc_context(ctx);
    ctx->context = pt;
    ctx->handler = trust_resource_compute;
    ctx->interval = pt->wTimerDelay;
    ctx->quit_flag = &(pt->quit_flag);
    return eal_timer_create2(&(pt->timer), ctx);
}
#if defined(USE_MMTIMER)

void CALLBACK trust_thread_proc(UINT wTimerID, UINT msg,
                                DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    UNREFERENCED_PARAM(wTimerID);
    UNREFERENCED_PARAM(msg);
    UNREFERENCED_PARAM(dw1);
    UNREFERENCED_PARAM(dw2);

    lm_trust_t*   pt = (lm_trust_t*)dwUser;
    trust_resource_compute(pt);
}

int stop_trust_thread(lm_trust_t* pm)
{
    timeKillEvent(pm->wTimerID);
    timeEndPeriod(pm->wTimerRes);

    pm->wTimerID = 0;
    pm->wTimerRes = 0;
    return 0;

}

int create_trust_thread(lm_trust_t* pt)
{

    TIMECAPS tc;

    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        DWORD err = GetLastError();
        lmice_critical_print("Create time thread failed[%u]\n", err);
        return 1;
    }

    pt->wTimerRes = min(max(tc.wPeriodMin, TRUST_RESOLUTION_MSEC), tc.wPeriodMax);
    pt->wTimerDelay = max(pt->wTimerRes, TRUST_PERIOD_MSEC);

    timeBeginPeriod(pt->wTimerRes);

    pt->wTimerID = timeSetEvent(
                pt->wTimerDelay,                      // delay
                pt->wTimerRes,                      // resolution (global variable)
                trust_thread_proc,                // callback function
                (DWORD_PTR)pt,                      // user data
                TIME_PERIODIC );                // single timer event
    if(! pt->wTimerID)
        return 1;
    else
        return 0;
}
#endif

#elif defined(__APPLE__)

static void trust_thread_proc(void* data)
{
    lm_trust_t*   pt = (lm_trust_t*)data;
    size_t pos = 0;
    int ret = 0;
    lm_worker_info_t *inst=NULL;
    lm_server_t *server = pt->server;
    ret = eal_spin_lock(&server->lock);
    for(inst = &server->worker, pos = 0; pos < DEFAULT_WORKER_SIZE; ++inst, ++pos) {

        int err;

        /* if it's empty, go check next */
        if( inst->inst_id == 0)
            continue;

        /* check version, only check LMICE_VERSION */
        if(inst->version != LMICE_VERSION)
            continue;

        /* check process state */
        if(inst->process_id != 0) {

            ret = kill(inst->process_id, 0);
            if(ret != 0) {
                err = errno;
                lmice_debug_print("process[%u] open failed[%d]\n", inst->process_id, err);
                memset(inst, 0, sizeof(lm_worker_info_t));
                inst->state = WORKER_DEAD;
                eal_event_awake(pt->efd);
                continue;
            }
        }

        /* check thread state */
        if(inst->thread_id != 0) {
            pthread_t tid = (pthread_t)(void*)inst->thread_id;
            ret = pthread_kill(tid, 0);
            if(ret != 0) {
                err = errno;
                lmice_debug_print("worker thread[%llu] open failed[%d]\n", inst->thread_id, err);
                memset(inst, 0, sizeof(lm_worker_info_t));
                inst->state = WORKER_DEAD;
                eal_event_awake(pt->efd);
                continue;
            }
        }
        /*
        if(inst->process_id == 0 && inst->thread_id == 0){
            memset(inst, 0, sizeof(lm_worker_info_t));
        }
        */
    }
    eal_spin_unlock(&server->lock);

}

int create_trust_thread(lm_trust_t* pt)
{
    lm_timer_ctx_t *ctx = NULL;
    pt->wTimerDelay = TRUST_PERIOD_MSEC * 10000ull;
    pt->wTimerRes = TRUST_RESOLUTION_MSEC * 10000ull;
    /*
 *     eal_timer_create(&(pt->wTimerID), "LMiced Trust Thread", &(pt->wTimerDelay), &(pt->wTimerRes),
                     trust_thread_proc, (void*)pt);
    if(pt->wTimerID)
        return 0;
    else
        return 1;
*/
    eal_timer_malloc_context(ctx);
    ctx->context = pt;
    ctx->handler = trust_thread_proc;
    ctx->interval = pt->wTimerDelay;
    ctx->quit_flag = &(pt->quit_flag);
    return eal_timer_create2(&(pt->timer), ctx);
}


#endif

int stop_trust_thread(lm_trust_t* pt)
{
    eal_timer_destroy2(pt);

    return 0;
}
