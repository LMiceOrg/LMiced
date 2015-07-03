#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_hash.h"
#include "eal/lmice_eal_spinlock.h"
#include "eal/lmice_trace.h"
#include "eal/lmice_eal_hash.h"

#include "resource_manage.h"
#include "rtspace.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>


int create_server_resource(lm_shm_res_t *server);
int destroy_server_resource(lm_shm_res_t* server);


int forceinline open_action_resource(lm_action_res_t* act)
{
    int ret = 0;
    act->active = 1;
    return ret;
}

int forceinline close_action_resource(lm_action_res_t* act)
{
    int ret = 0;
    act->active = 0;
    return ret;
}

void forceinline maintain_worker_action_resource(lm_worker_res_t* worker)
{
    size_t i=0;
    lm_action_res_t *act = NULL;
    for(i=0; i< 128; ++i)
    {
        act = worker->action +i;
        if(act->active == 0
                && act->info->inst_id == 0)
        {
            /* this is an empty timer, worker never use it */
            continue;
        }
        else if(act->active != 0
                && act->info->inst_id == 0)
        {
            /* the message is destroyed at worker side */
            /* close it at server side */
            close_action_resource(act);

        }
        else if(act->active == 0
                && act->info->inst_id != 0)
        {
            /* the timer is created at worker side */
            /* open it at server side */
            open_action_resource(act);
        }
        /* else the message synced at both server side and client side */

    }
}

int forceinline open_timer_resource(lm_res_param_t* pm, lm_timer_res_t* timer)
{
    int ret = 0;
    lm_res_task_t task;

    UNREFERENCED_PARAM(pm);

    task.type = LM_RES_TASK_ADD_TIMER;
    task.pval = timer;
    printf("%p, %ud\n", task.pval, timer->info->size);
    timer->active = LM_TIMER_RUNNING;
    ret = set_resource_task(&task);
    return ret;
}


int forceinline close_timer_resource(lm_res_param_t* pm, lm_timer_res_t* timer)
{
    lm_res_task_t task;
    UNREFERENCED_PARAM(pm);
    task.type = LM_RES_TASK_DEL_TIMER;
    task.pval = timer;
    timer->active = LM_TIMER_DELETE;
    set_resource_task(&task);

    lmice_debug_print("close_timer_resource\n");
    return 0;

}

void forceinline maintain_worker_timer_resource(lm_worker_res_t* worker, lm_res_param_t* pm)
{
    size_t i=0;
    lm_timer_res_t *timer = NULL;
    lm_timer_info_t *info = NULL;
    for(i=0; i< 128; ++i)
    {
        timer = worker->timer +i;
        info = timer->info;
        if(timer->active == LM_TIMER_NOTUSE
                && timer->info->inst_id == 0)
        {
            /* this is an empty timer, worker never use it */
            continue;
        }
        else if(timer->active == LM_TIMER_RUNNING
                && timer->info->inst_id == 0)
        {
            /* the message is destroyed at worker side */
            /* close it at server side */
            close_timer_resource(pm, timer);

        }
        else if(timer->active == LM_TIMER_NOTUSE
                && timer->info->inst_id != 0)
        {
            /* the timer is created at worker side */
            /* open it at server side */
            open_timer_resource(pm, timer);

        }
        /* else the message synced at both server side and client side */

    }
}

int forceinline open_message_resource(lm_mesg_res_t* mesg)
{
    int ret = 0;
    lmice_shm_t shm;

    eal_shm_zero(&shm);
    eal_shm_hash_name(mesg->info->inst_id, shm.name);
    ret = eal_shm_open_readwrite(&shm);
    if(ret != 0)
    {
        lmice_critical_print("open shm[%s] failed[%d]\n", shm.name, ret);
        return ret;
    }

    mesg->addr = shm.addr;
    mesg->sfd = shm.fd;

    return 0;
}

int forceinline close_message_resource(lm_mesg_res_t* mesg)
{
    int ret= 0;

    ret = eal_shm_close(mesg->sfd, mesg->addr);
    mesg->addr = 0;
    mesg->sfd = 0;

    return ret;
}

void forceinline maintain_worker_message_resource(lm_worker_res_t* worker, lm_res_param_t* pm)
{
    size_t i=0;
    lm_mesg_res_t *mesg = NULL;

    UNREFERENCED_PARAM(pm);

    for(i=0; i< 128; ++i)
    {
        mesg = worker->mesg +i;
        if(mesg->addr == 0
                && mesg->info->inst_id == 0)
        {
            /* this is an empty message, worker never use it */
            continue;
        }
        else if(mesg->addr != 0
                && mesg->info->inst_id == 0)
        {
            /* the message is destroyed at worker side */
            /* close it at server side */
            close_message_resource(mesg);

        }
        else if(mesg->addr == 0
                && mesg->info->inst_id != 0)
        {
            /* the message is created at worker side */
            /* open it at server side */
            open_message_resource(mesg);

        }
        /* else the message synced at both server side and client side */

    }
}

void forceinline init_worker_resource(lm_worker_res_t* worker)
{
    size_t i = 0;
    lm_worker_t* inst = (lm_worker_t*)worker->res.addr;
    lmice_critical_print("init worker resource %p\n", inst);

    for(i=0; i<128; ++i)
    {
        worker->mesg[i].addr = 0;
        worker->mesg[i].info = NULL;
    }
    for(i=0; i<128; ++i)
    {
        worker->timer[i].active = LM_TIMER_NOTUSE;
        worker->timer[i].info = NULL;
        worker->timer[i].worker = worker;
    }
    for(i=0; i<128; ++i)
    {
        worker->action[i].active = 0;
        worker->action[i].info = NULL;
    }
}

int forceinline open_worker_resource(lm_worker_res_t *worker)
{
    int ret;
    lmice_shm_t shm;
    lmice_event_t evt;
    eal_shm_zero(&shm);
    eal_shm_hash_name(worker->info->inst_id, shm.name);
    ret = eal_shm_open_readwrite(&shm);
    if(ret != 0)
    {
        lmice_critical_print("open work_shm[%s] failed[%d]\n", shm.name, ret);
        return ret;
    }


    eal_event_zero(&evt);
    eal_event_hash_name(worker->info->inst_id, evt.name);
    ret = eal_event_open(&evt);
    if(ret != 0)
    {
        lmice_critical_print("open worker_event[%s] failed[%d]\n", evt.name, ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    worker->res.addr = shm.addr;
    worker->res.sfd = shm.fd;
    worker->res.efd = evt.fd;
    init_worker_resource(worker);
    return 0;
}

int forceinline close_worker_resource(lm_worker_res_t *worker, lm_res_param_t* pm)
{
    int ret = 0;
    lm_res_task_t task;
    size_t i = 0;
    lm_mesg_res_t *mesg = NULL;
    lm_timer_res_t *timer = NULL;
    lm_action_res_t *act = NULL;

    UNREFERENCED_PARAM(pm);

    /* close message, timer, action */
    for(i=0; i<128; ++i)
    {
        mesg = worker->mesg +i;
        close_message_resource(mesg);
    }
    for(i=0; i<128; ++i)
    {
        timer = worker->timer +i;
        timer->active = LM_TIMER_NOTUSE;

    }
    for(i=0; i<128; ++i)
    {
        act = worker->action +i;
        close_action_resource(act);
    }

    ret = eal_shm_close(worker->res.sfd, worker->res.addr);
    worker->res.sfd = 0;
    worker->res.addr = 0;

    task.type = LM_RES_TASK_DEL_WORKER;
    task.pval = worker;
    ret = set_resource_task(&task);
    return ret;
}

void forceinline maintain_worker_resource(lm_res_param_t* pm)
{
    size_t i = 0;
    lm_worker_res_t *worker = NULL;
    lm_server_t* server = NULL;

    lmice_debug_print("call maintain worker resource\n");

    server = (lm_server_t*)pm->res_server.addr;
    eal_spin_lock(&server->board.lock);
    for(i=0; i<DEFAULT_WORKER_SIZE; ++i)
    {
        worker = pm->res_worker +i;

        /* no resource changing */
        if(worker->info->state == WORKER_RUNNING)
            continue;

        if(worker->res.addr == 0
                && worker->info->inst_id == 0)
        {
            /* the worker is never used */
            continue;
        }
        else if(worker->res.addr != 0
                && worker->info->inst_id == 0)
        {
            /* the worker is closed */
            /* so close it at server side */
            lmice_debug_print("the worker is closed\n");
            close_worker_resource(worker, pm);
            continue;
        }
        else if(worker->res.addr == 0
                && worker->info->inst_id != 0)
        {
            /* a new worker is created */
            /* so open it at server side */
            open_worker_resource(worker);
            lmice_critical_print("open_worker_resource[%d:%llu]\n", worker->info->process_id, worker->info->thread_id);


        }
        /* else the worker synced at both server and worker side */

        if(worker->info->state == WORKER_MODIFIED)
        {
            lmice_debug_print("sync worker[%d:%llu] message\n", worker->info->process_id, worker->info->thread_id);

            /* sync message resource timer and action */
            maintain_worker_message_resource(worker, pm);
            maintain_worker_timer_resource(worker, pm);
            maintain_worker_action_resource(worker);
            worker->info->state = WORKER_RUNNING;
        }

    }
    eal_spin_unlock(&server->board.lock);
}
#ifdef _WIN32

DWORD WINAPI worker_maintain_thread_proc( LPVOID lpParameter)
{
    lm_res_param_t* pm = (lm_res_param_t*)lpParameter;
    for(;;)
    {
        //lmice_debug_print("call worker_maintain_thread_proc\n");
        DWORD code = WaitForSingleObject(pm->res_server.efd,
                                         INFINITE);
        if(code == WAIT_FAILED
                || code == WAIT_TIMEOUT)
            break;
        maintain_worker_resource(pm);
    }
    return 0;
}



/**
 * @brief create_instance_thread
 * 创建线程以维护工作实例资源索引
 * @return
 */
int create_worker_maintain_thread(void* param)
{
    DWORD tid;
    HANDLE trd;
    trd = CreateThread( NULL,
                        0,
                        worker_maintain_thread_proc,
                        param,
                        0,
                        &tid);
    if(trd == NULL)
    {
        DWORD err = GetLastError();
        lmice_critical_print("Create instance maintain thread failed[%u]\n", err);
        return 1;
    }
    return 0;
}

#elif defined(__APPLE__)

static void* worker_maintain_thread_proc(void* param)
{
    int ret = 0;
    lm_res_param_t* pm = (lm_res_param_t*)param;
    for(;;) {
        ret = eal_event_wait_one(pm->res_server.efd);
        if(ret == 0)
            maintain_worker_resource(pm);
        else
            break;
    }
    return NULL;

}

static int create_worker_maintain_thread(void* param)
{
    pthread_t pt;
    pthread_create(&pt,
                   NULL,
                   worker_maintain_thread_proc,
                   param);
    return 0;
}

#endif

int create_server_resource(lm_shm_res_t* server)
{
    int ret = 0;
    uint64_t hval = 0;
    eal_pid_t m_pid = 0;
    lm_server_t *m_server = NULL;
    lmice_event_t   evt;
    lmice_shm_t     shm;

    /*获取当前进程ID*/
    m_pid = getpid();

    /*创建平台公共区域共享内存*/
    eal_shm_zero(&shm);
    shm.size = DEFAULT_SHM_SIZE * 2;
    hval = eal_hash64_fnv1a(BOARD_SHMNAME, sizeof(BOARD_SHMNAME)-1);
    eal_shm_hash_name(hval, shm.name);
    ret = eal_shm_create(&shm);
    if(ret != 0)
    {
        eal_shm_destroy(&shm);
        lmice_critical_print("create server shm failed[%d]\n", ret);
        return ret;
    }
    lmice_critical_print("create server shm\n");

    /*创建公共区域管理事件*/
    eal_event_zero(&evt);
    eal_event_hash_name(hval, evt.name);
    ret = eal_event_create(&evt);
    if(ret != 0)
    {
        eal_event_destroy(&evt);
        lmice_critical_print("create server event failed[%d]\n", ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    m_server = (lm_server_t*)((void*)(shm.addr));
    memset(m_server, 0, sizeof(lm_server_t));
    /* update server: board */
    m_server->board.version = LMICE_VERSION;
    m_server->board.state = 0;
    m_server->board.size = shm.size;
    m_server->board.next_id = 0;
    m_server->board.lock = 0;
    m_server->board.inst_id = hval;
    m_server->board.type_id = hval;
    /* update server:time */
    /* update server:worker_info */
    m_server->worker_capacity = (shm.size - ( (size_t)&(((lm_server_t*)0)->worker) - (size_t)&(((lm_server_t*)0)->board) )) / sizeof( lm_worker_info_t );
    m_server->worker_size = 0;

    /* update server_res */
    server->addr = shm.addr;
    server->efd = evt.fd;
    server->sfd = shm.fd;

    return ret;
}

int destroy_server_resource(lm_shm_res_t *server)
{
    int ret = 0;
    lmice_shm_t shm;
    lmice_event_t evt;
    uint64_t hval = 0;
    /* destroy client resource first */

    hval = eal_hash64_fnv1a(BOARD_SHMNAME, sizeof(BOARD_SHMNAME)-1);

    /* close server event and shm*/
    eal_event_zero(&evt);
    eal_event_hash_name(hval, evt.name);
    evt.fd = server->efd;
    /* lmice_critical_print("eal_event_close[%s] %p\n",evt.name, evt.fd);
    */
    ret = eal_event_destroy(&evt);
    if(ret != 0) {
        lmice_critical_print("close server event failed[%d]\n", ret);
    }

    eal_shm_zero(&shm);
    shm.size = DEFAULT_SHM_SIZE * 2;
    eal_shm_hash_name(hval, shm.name);
    shm.fd = server->sfd;
    shm.addr = server->addr;
    /*
     * lmice_critical_print("eal_shm_close[%s] %d",shm.name, shm.fd);
    */
    ret = eal_shm_destroy(&shm);
    if(ret != 0) {
        lmice_critical_print("close server shm failed[%d]\n", ret);
    }

    return ret;
}

void forceinline init_tmlist_resource(lm_timer_res_t** tlist)
{
    /* create the next-pos element (for control) */
    tlist[TIMER_LIST_NEXT_POS]= (lm_timer_res_t*)malloc(sizeof(lm_timer_res_t));

    /* reset control element value */
    memset(tlist[TIMER_LIST_NEXT_POS], 0, sizeof(lm_timer_res_t) );
}

forceinline void finit_tmlist_resource(lm_timer_res_t** tlist) {
    free( tlist[TIMER_LIST_NEXT_POS] );
}

forceinline void finit_server_resource(lm_res_param_t* pm) {
    finit_tmlist_resource(pm->ticker_duelist);
    finit_tmlist_resource(pm->ticker_worklist1);
    finit_tmlist_resource(pm->ticker_worklist2);
    finit_tmlist_resource(pm->ticker_worklist3);
    finit_tmlist_resource(pm->ticker_worklist4);
    finit_tmlist_resource(pm->timer_duelist);
    finit_tmlist_resource(pm->timer_worklist1);
    finit_tmlist_resource(pm->timer_worklist2);
    finit_tmlist_resource(pm->timer_worklist3);
    finit_tmlist_resource(pm->timer_worklist4);
}

void forceinline init_server_resource(lm_res_param_t* pm)
{
    size_t i = 0;
    lm_server_t *server = (lm_server_t *)pm->res_server.addr;
    pm->tm_param.pt = &server->tm;
    memset(pm->res_worker, 0, sizeof(pm->res_worker));
    lmice_critical_print("init_server_resource\n");
    for(i=0; i<DEFAULT_WORKER_SIZE; ++i)
    {
        pm->res_worker[i].info = &server->worker + i;
    }

    init_tmlist_resource(pm->ticker_duelist);
    init_tmlist_resource(pm->ticker_worklist1);
    init_tmlist_resource(pm->ticker_worklist2);
    init_tmlist_resource(pm->ticker_worklist3);
    init_tmlist_resource(pm->ticker_worklist4);
    init_tmlist_resource(pm->timer_duelist);
    init_tmlist_resource(pm->timer_worklist1);
    init_tmlist_resource(pm->timer_worklist2);
    init_tmlist_resource(pm->timer_worklist3);
    init_tmlist_resource(pm->timer_worklist4);
}

forceinline int create_server_worker_resource(lm_res_param_t* pm)
{
    int ret = 0;
    lmice_event_t   evt;
    lmice_shm_t  shm;
    lm_worker_res_t * worker = pm->res_worker;
    lm_worker_t* wk = NULL;
    uint64_t worker_id;
    uint64_t type_id;
    eal_pid_t pid = getpid();
    eal_tid_t tid = 0;
    type_id = eal_hash64_fnv1a(CLIENT_SHMNAME, sizeof(CLIENT_SHMNAME)-1);
    worker_id = eal_hash64_more_fnv1a(&pid, sizeof(pid),type_id);
    worker_id = eal_hash64_more_fnv1a(&tid, sizeof(tid), worker_id);

    eal_shm_zero(&shm);
    shm.size = DEFAULT_SHM_SIZE*16;
    eal_shm_hash_name(worker_id, shm.name);
    ret = eal_shm_create(&shm);
    if(ret != 0)
    {
        lmice_critical_print("create_worker_resource call eal_shm_create[%s] size(%d) failed[%d]\n", shm.name, shm.size, ret);
        return ret;
    }

    eal_event_zero(&evt);
    eal_event_hash_name(worker_id, evt.name);
    ret = eal_event_create(&evt);
    if(ret != 0)
    {
        lmice_critical_print("create_worker_resource call eal_event_create failed[%d]\n", ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    /* maintain worker's resource manage */
    worker->res.addr = shm.addr;
    worker->res.sfd = shm.fd;
    worker->res.efd = evt.fd;

    /* maintain worker's shared memory */
    wk = (lm_worker_t*)shm.addr;
    memset(wk, 0, sizeof(shm.size));
    wk->board.version = LMICE_VERSION;
    wk->board.state = WORKER_RUNNING;
    wk->board.size = shm.size;
    wk->board.inst_id = worker_id;
    wk->board.type_id = worker_id;
    wk->res_capacity = (shm.size - ( (size_t)&(((lm_worker_t*)0)->res) - (size_t)&(((lm_worker_t*)0)->board) ))/ sizeof(lm_res_info_t);

    /* maintain server's worker info */
    worker->info->version = LMICE_VERSION;
    worker->info->state = WORKER_RUNNING;
    worker->info->process_id = pid;
    worker->info->thread_id = tid;
    worker->info->type_id = type_id;
    worker->info->inst_id = worker_id;

    return 0;
}

int destroy_server_worker_resource(lm_shm_res_t * ipc)
{
    int ret = 0;
    ret = eal_event_close(ipc->efd);
    ret |= eal_shm_close(ipc->sfd, ipc->addr);

    return ret;
}

int create_resource_service(lm_res_param_t* pm)
{
    int ret = 0;
    lm_shm_res_t *m_resource = &pm->res_server;


    lmice_debug_print("enter create_resource_service\n");

    /* 创建服务端共享资源区 */
    memset(m_resource, 0, sizeof(lm_shm_res_t));
    ret = create_server_resource(m_resource);
    if(ret != 0)
    {
        lmice_critical_print("Create server resource shm failed[%d]\n", ret);
        return 1;
    }
    /* 初始化资源信息*/    
    init_server_resource(pm);

    /* 创建server端工作实例 */
    ret = create_server_worker_resource(pm);
    if(ret != 0)
    {
        lmice_critical_print("Create server side worker shm failed[%d]\n", ret);
        return 1;
    }

    /* 启动工作实例资源维护服务 */
    create_worker_maintain_thread(pm);

    return ret;
}

int destroy_resource_service(lm_res_param_t* pm)
{
    int ret = 0;
    lm_shm_res_t *m_resource = &pm->res_server;
    lm_shm_res_t *m_worker = &pm->res_worker[0].res;

    ret = close_worker_resource(pm->res_worker, pm);

    ret |= destroy_server_worker_resource(m_worker);

    finit_server_resource(pm);

    ret |= destroy_server_resource(m_resource);

    return ret;
}


static lm_res_task_t g_restask[128];
static volatile int64_t g_reslock = 0;
int peek_resource_task(lm_res_task_t* task)
{
    int ret = 0;
    ret = eal_spin_trylock(&g_reslock);
    if(ret !=0)
        return ret;

    if(g_restask[0].type != LM_RES_TASK_NOTUSE)
    {
        memcpy(task, g_restask, sizeof(lm_res_task_t));
        memmove(g_restask, g_restask+1, 127*sizeof(lm_res_task_t));
        g_restask[127].type = LM_RES_TASK_NOTUSE;

    }

    eal_spin_unlock(&g_reslock);
    return 0;
}
#define RES_TASKLIST_SIZE 128
int set_resource_task(lm_res_task_t* task)
{
    int ret = 0;
    int i;
    /* 锁定任务队列 */
    ret = eal_spin_trylock(&g_reslock);
    if(ret !=0)
        return ret;

    ret = 1;
    /* 遍历任务队列 */
    for(i= 0; i < RES_TASKLIST_SIZE; ++i) {
        /* 判定任务队列当前位置是否可用 */
        if(g_restask[i].type == LM_RES_TASK_NOTUSE) {
            memcpy(g_restask+i, task, sizeof(lm_res_task_t));
            ret = 0;
            break;
        }
    }
    /* 解除任务队列锁定 */
    eal_spin_unlock(&g_reslock);
    return ret;
}

int append_worker_to_res(lm_res_param_t* pm, lm_worker_res_t* worker)
{
    UNREFERENCED_PARAM(pm);
    UNREFERENCED_PARAM(worker);
    return 0;
}

int remove_worker_from_res(lm_res_param_t* pm, lm_worker_res_t* worker)
{
    int ret = 0;

    ret = remove_timer_by_worker(pm, worker);

    worker->info->inst_id = 0;
    worker->info->state = 0;
    return ret;
}
