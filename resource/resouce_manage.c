#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_hash.h"
#include "eal/lmice_trace.h"

#include "resource_manage.h"
#include "rtspace.h"

#include <sglib.h>

#include <stdio.h>
#include <errno.h>


int create_server_resource(lm_inst_res_t *server);
int destroy_server_resource(lm_inst_res_t* server);

void forceinline instance_resource_maintain(lm_res_param_t* pm)
{
    int ret;
    size_t i = 0;
    size_t j = 0;
    lm_inst_res_t *server_res = &pm->res_server;
    lm_server_t * server =  (lm_server_t*)((void*)(server_res->addr));
    lm_worker_t *worker = NULL;
    lm_worker_info_t *worker_info = NULL;
    lm_worker_res_t *worker_res = NULL;


    lmice_shm_t shm;
    lmice_event_t evt;

    for(i=0; i<DEFAULT_CLIENT_SIZE; ++i)
    {
        worker_res = pm->res_worker +i;
        worker_info = &server->worker + i;

        if(worker_info->process_id == 0
                && worker_info->thread_id == 0)
            continue;

        /* not found worker res, so this is new worker, register it */
        if(worker_res->process_id == 0
                && worker_res->thread_id == 0)
        {

            eal_shm_zero(&shm);
            eal_shm_hash_name(worker_info->instance_id, shm.name);
            ret = eal_shm_open_readonly(&shm);
            if(ret != 0)
            {
                lmice_critical_print("open client_shm[%s] failed[%d]\n", shm.name, ret);
                continue;
            }
            worker_res->res.addr = shm.addr;
            worker_res->res.sfd = shm.fd;

            eal_event_zero(&evt);
            eal_event_hash_name(worker_info->instance_id, evt.name);
            ret = eal_event_open(&evt);
            if(ret != 0)
            {
                lmice_critical_print("open client_event[%s] size(%d) failed\n", evt.name, ret);
                eal_shm_close(shm.fd, shm.addr);
                continue;
            }
            worker_res->res.efd = evt.fd;

            worker_res->process_id = worker_info->process_id;
            worker_res->thread_id = worker_info->thread_id;

        }
        /* update worker res */
        worker = (lm_worker_t*)worker_res->res.addr;
        /* message resource */

    }
}

DWORD WINAPI instance_thread_proc( LPVOID lpParameter)
{
    HANDLE hWaitEvents[2];
    LARGE_INTEGER due;
    /* millisecond */
    LONG period = 30;
    BOOL ret= 1;
    due.QuadPart = -1;

    hWaitEvents[0] = CreateWaitableTimer(NULL,
                                         FALSE,
                                         NULL);
    ret = SetWaitableTimer(hWaitEvents[0],
            &due,
            period,
            NULL,
            NULL,
            FALSE);
    if(ret == 0)
    {
        DWORD err = GetLastError();
        lmice_critical_print("Create waitable timer failed[%u]\n", err);
        return 1;
    }
    while(1)
    {
        DWORD code = WaitForMultipleObjects(2,
                               hWaitEvents,
                               TRUE,
                               INFINITE);
        if(code == WAIT_FAILED)
            break;

    }

    CloseHandle(hWaitEvents[0]);
    return 0;
}


/**
 * @brief create_instance_thread
 * 创建线程以维护工作实例资源索引
 * @return
 */
int create_instance_thread(void* param)
{
    DWORD tid;
    HANDLE trd;
    trd = CreateThread( NULL,
                        0,
                        instance_thread_proc,
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


int create_server_resource(lm_inst_res_t* server)
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
    m_server = (lm_server_t*)((void*)(shm.addr));
    m_server->event_id = hval;
    m_server->lock = 0;
    m_server->size = shm.size;
    m_server->version = LMICE_VERSION;
    m_server->next_id = 0;

    /*创建公共区域管理事件*/
    eal_event_zero(&evt);
    eal_event_hash_name(hval, evt.name);
    ret = eal_event_create(&evt);
    if(ret != 0)
    {
        lmice_critical_print("create server event failed[%d]\n", ret);
    }

    server->addr = shm.addr;
    server->efd = evt.fd;
    server->sfd = shm.fd;
    return ret;
}

int destroy_server_resource(lm_inst_res_t *server)
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

int create_resource_service(lm_res_param_t* res)
{
    int ret = 0;
    lm_inst_res_t *m_resource = &res->res_server;
    lm_time_param_t *m_time = &res->tm_param;
    lm_server_t *m_server = NULL;

    /* 创建服务端共享资源区 */
    memset(m_resource, 0, sizeof(lm_worker_res_t));
    ret = create_server_resource(m_resource);
    if(ret != 0)
    {
        lmice_critical_print("Create server resource shm failed[%d]\n", ret);
        return 1;
    }

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

    return ret;
}

int destroy_resource_service(lm_res_param_t* res)
{
    int ret = 0;
    lm_time_param_t *m_time = &res->tm_param;
    lm_inst_res_t *m_resource = &res->res_server;

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
