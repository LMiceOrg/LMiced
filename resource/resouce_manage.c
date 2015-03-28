#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_hash.h"
#include "eal/lmice_eal_spinlock.h"
#include "eal/lmice_trace.h"

#include "resource_manage.h"
#include "rtspace.h"

#include <sglib.h>

#include <stdio.h>
#include <errno.h>


int create_server_resource(lm_shm_res_t *server);
int destroy_server_resource(lm_shm_res_t* server);

int forceinline open_action_resource(lm_action_res_t* act)
{
    int ret = 0;
    lmice_event_t evt;
    eal_event_zero(&evt);
    eal_event_hash_name(act->info->inst_id, evt.name);
    ret = eal_event_open(&evt);
    if(ret != 0)
    {
        lmice_critical_print("open event[%s] failed[%d]\n", evt.name, ret);
    }
     return ret;
}

int forceinline close_action_resource(lm_action_res_t* act)
{
    int ret = 0;
    ret = eal_event_close(act->efd);
    act->efd = 0;
    return ret;
}

void forceinline maintain_worker_action_resource(lm_worker_res_t* worker)
{
    size_t i=0;
    lm_action_res_t *act = NULL;
    for(i=0; i< 128; ++i)
    {
        act = worker->action +i;
        if(act->efd == 0
                && act->info->inst_id == 0)
        {
            /* this is an empty timer, worker never use it */
            continue;
        }
        else if(act->efd != 0
                && act->info->inst_id == 0)
        {
            /* the message is destroyed at worker side */
            /* close it at server side */
            close_action_resource(act);

        }
        else if(act->efd == 0
                && act->info->inst_id != 0)
        {
            /* the timer is created at worker side */
            /* open it at server side */
            open_action_resource(act);
        }
        /* else the message synced at both server side and client side */

    }
}

int forceinline open_timer_resource(lm_timer_res_t* timer)
{
    int ret = 0;
    lmice_event_t evt;
    eal_event_zero(&evt);
    eal_event_hash_name(timer->info->inst_id, evt.name);
    ret = eal_event_open(&evt);
    if(ret != 0)
    {
        lmice_critical_print("open event[%s] failed[%d]\n", evt.name, ret);
    }
     return ret;
}

int forceinline close_timer_resource(lm_timer_res_t* timer)
{
    int ret = 0;
    ret = eal_event_close(timer->efd);
    timer->efd = 0;
    return ret;
}

void forceinline maintain_worker_timer_resource(lm_worker_res_t* worker)
{
    size_t i=0;
    lm_timer_res_t *timer = NULL;
    for(i=0; i< 128; ++i)
    {
        timer = worker->timer +i;
        if(timer->efd == 0
                && timer->info->inst_id == 0)
        {
            /* this is an empty timer, worker never use it */
            continue;
        }
        else if(timer->efd != 0
                && timer->info->inst_id == 0)
        {
            /* the message is destroyed at worker side */
            /* close it at server side */
            close_timer_resource(timer);

        }
        else if(timer->efd == 0
                && timer->info->inst_id != 0)
        {
            /* the timer is created at worker side */
            /* open it at server side */
            open_timer_resource(timer);
        }
        /* else the message synced at both server side and client side */

    }
}

int forceinline open_message_resource(lm_mesg_res_t* mesg)
{
    int ret = 0;
    lmice_shm_t shm;
    lmice_event_t evt;

    eal_shm_zero(&shm);
    eal_shm_hash_name(mesg->info->inst_id, shm.name);
    ret = eal_shm_open_readwrite(&shm);
    if(ret != 0)
    {
        lmice_critical_print("open shm[%s] failed[%d]\n", shm.name, ret);
        return ret;
    }

    eal_event_zero(&evt);
    eal_event_hash_name(mesg->info->inst_id, evt.name);
    ret = eal_event_open(&evt);
    if(ret != 0)
    {
        lmice_critical_print("open event[%s] failed[%d]\n", evt.name, ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    mesg->res.addr = shm.addr;
    mesg->res.sfd = shm.fd;
    mesg->res.efd = evt.fd;

    return 0;
}

int forceinline close_message_resource(lm_mesg_res_t* mesg)
{
    int ret= 0;

    ret = eal_shm_close(mesg->res.sfd, mesg->res.addr);
    ret = eal_event_close(mesg->res.efd);

    memset(&mesg->res, 0, sizeof(lm_shm_res_t));

    return ret;
}

void forceinline maintain_worker_message_resource(lm_worker_res_t* worker)
{
    size_t i=0;
    lm_mesg_res_t *mesg = NULL;
    for(i=0; i< 128; ++i)
    {
        mesg = worker->mesg +i;
        if(mesg->res.sfd == 0
                && mesg->info->inst_id == 0)
        {
            /* this is an empty message, worker never use it */
            continue;
        }
        else if(mesg->res.sfd != 0
                && mesg->info->inst_id == 0)
        {
            /* the message is destroyed at worker side */
            /* close it at server side */
            close_message_resource(mesg);

        }
        else if(mesg->res.sfd == 0
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
    for(i=0; i<128; ++i)
    {
        worker->mesg[i].info = &inst->mesg[i];
        worker->timer[i].info = &inst->timer[i];
        worker->action[i].info = &inst->action[i];
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

int forceinline close_worker_resource(lm_worker_res_t *worker)
{
    int ret = 0;
    size_t i = 0;
    lm_mesg_res_t *mesg = NULL;

    /* close message, timer, action */
    for(i=0; i<128; ++i)
    {
        mesg = worker->mesg +i;
        close_message_resource(mesg);
    }

    ret = eal_shm_close(worker->res.sfd, worker->res.addr);
    ret = eal_event_close(worker->res.efd);

    memset(&worker->res, 0, sizeof(lm_shm_res_t));
    return ret;
}

void forceinline maintain_worker_resource(lm_res_param_t* pm)
{
    size_t i = 0;
    lm_worker_res_t *worker = NULL;
    lmice_debug_print("call maintain worker resource\n");
    lm_server_t* server = (lm_server_t*)pm->res_server.addr;
    eal_spin_lock(&server->lock);
    for(i=0; i<DEFAULT_CLIENT_SIZE; ++i)
    {
        worker = pm->res_worker +i;

        /* no resource changing */
        if(worker->info->state == WORKER_RUNNING)
            continue;

        if(worker->res.sfd == 0
                && worker->info->inst_id == 0)
        {
            /* the worker is never used */
            continue;
        }
        else if(worker->res.sfd != 0
                && worker->info->inst_id == 0)
        {
            /* the worker is closed */
            /* so close it at server side */
            close_worker_resource(worker);
            continue;
        }
        else if(worker->res.sfd == 0
                && worker->info->inst_id != 0)
        {
            /* a new worker is created */
            /* so open it at server side */
            open_worker_resource(worker);


        }
        /* else the worker synced at both server and worker side */

        if(worker->info->state == WORKER_MODIFIED)
        {
            /* sync message resource timer and action */
            maintain_worker_message_resource(worker);
            maintain_worker_timer_resource(worker);
            maintain_worker_action_resource(worker);
            worker->info->state = WORKER_RUNNING;
        }

    }
    eal_spin_unlock(&server->lock);
}

DWORD WINAPI worker_maintain_thread_proc( LPVOID lpParameter)
{
    lm_res_param_t* pm = (lm_res_param_t*)lpParameter;
    while(1)
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


int create_server_resource(lm_shm_res_t* server)
{
    int ret = 0;
    uint64_t hval = 0;
    pid_t m_pid = 0;
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
        lmice_critical_print("create server event failed[%d]\n", ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    m_server = (lm_server_t*)((void*)(shm.addr));
    m_server->event_id = hval;
    m_server->lock = 0;
    m_server->size = shm.size;
    m_server->version = LMICE_VERSION;
    m_server->next_id = 0;

    server->addr = shm.addr;
    server->efd = evt.fd;
    server->sfd = shm.fd;
    return ret;
}

int destroy_server_resource(lm_shm_res_t *server)
{
    int ret = 0;
    /* destroy client resource first */

    /* close server event and shm*/
    ret = eal_event_close(server->efd);
    if(ret != 0)
        lmice_critical_print("close server event failed[%d]\n", ret);

    ret = eal_shm_close(server->sfd, server->addr);
    if(ret != 0)
        lmice_critical_print("close server shm failed[%d]\n", ret);

    return ret;
}

void forceinline init_server_resource(lm_res_param_t* pm)
{
    size_t i = 0;
    lm_server_t *server = (lm_server_t *)pm->res_server.addr;
    pm->tm_param.pt = &server->tm;
    memset(pm->res_worker, 0, sizeof(pm->res_worker));
    lmice_critical_print("init_server_resource\n");
    for(i=0; i<DEFAULT_CLIENT_SIZE; ++i)
    {
        pm->res_worker[i].info = &server->worker + i;
    }
}

int create_resource_service(lm_res_param_t* pm)
{
    int ret = 0;
    lm_shm_res_t *m_resource = &pm->res_server;
    lm_time_param_t *m_time = &pm->tm_param;
    lm_server_t *m_server = NULL;

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
    printf("init server resource\n");
    init_server_resource(pm);

    /* 启动时间维护服务 */
    memset(m_time, 0, sizeof(lm_time_param_t));
    m_server = (lm_server_t*)((void*)(m_resource->addr));
    m_time->pt = &m_server->tm;
    ret = create_time_thread(m_time);
    if(ret != 0)
    {
        lmice_critical_print("Create time service failed[%d]", ret);
    }

    /* 启动工作实例资源维护服务 */
    create_worker_maintain_thread(pm);

    return ret;
}

int destroy_resource_service(lm_res_param_t* pm)
{
    int ret = 0;
    lm_time_param_t *m_time = &pm->tm_param;
    lm_shm_res_t *m_resource = &pm->res_server;

    ret = stop_time_thread(m_time);
    ret = destroy_server_resource(m_resource);

    return ret;
}

//#include <jansson.h>

//const char* data_path = "data";

//int lmice_dump_resource_file()
//{

//    //TODO: server runtime --> instance scenario --> config --> data-path

//    //Dump scenario list json(id, type, owner)
//    const char* scen_temp = "\t{\n\t\tid:%lld\n\t\ttype:%lld\n\t\towner:%lld\n\t}\n";
//    char path[256]={0};
//    strcat(path, data_path);
//    strcat(path, "\\scenlist.log");
//    FILE* fp = fopen(path, "w");
//    if(!fp)
//        return errno;
//    setvbuf(fp, NULL, _IOFBF, 1024);
//    fwrite("[\n", 1, 2, fp);
//    fprintf(fp, scen_temp, 0, 1, 1);
//    fwrite("]\n", 1, 2, fp);
//    fflush(fp);
//    fclose(fp);

//    return 0;
//}

//int lmice_load_resource_file()
//{
//    char path[256]={0};
//    strcat(path, data_path);
//    strcat(path, "\\scenlist.log");

//    json_error_t err;
//    json_t *root;
//    FILE* fp = fopen(path, "r");
//    if(!fp)
//        return errno;

//    root = json_loadf(fp, 0, &err);

//    fclose(fp);

//}
