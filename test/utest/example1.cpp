extern "C"
{
#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_hash.h"
#include "eal/lmice_eal_shm.h"
#include "eal/lmice_eal_event.h"
#include "eal/lmice_trace.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_spinlock.h"
#include "rtspace.h"
#include "resource/resource_manage.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


class lmice_event_callback
{
public:
    virtual ~lmice_event_callback() {}
    virtual int operator() (void* pdata) = 0;
};

#define MAX_CLIENT_COUNT 200
#define ALL_INSTANCE (const char*)0
enum lmice_common
{
    TICK_NOW = 0,
    TIMER_NOW = 0,
    NO_TRUST_COMPUTION_SUPPORT = 0,
    NO_QOS_SUPPORT=0,
    DEFAULT_SESSION_ID = 0,
    PUBLISH_TYPE = 1,
    SUBSCRIBE_TYPE = 2,
    FREE_RESOURCE = 0,
    MODIFY_RESOURCE = 1,
    WORK_RESOURCE = 2,

    CUSTOM_TYPE = 3,
    INFINITY = -1,

    TIMER_EVENT_TYPE = 3,
    TICKER_EVENT_TYPE = 4,
    CUSTOM_EVENT_TYPE = 5,
    QOS_CONTROL_TYPE = 6,
    TRUST_CONTROL_TYPE
};


//struct lmice_resource_s
//{
//    uint32_t        type;   // publish subscribe
//    int32_t         state;  // 0 freed 1 modify 2 work
//    int64_t         time;   // state lock time
//    lmice_shm_t     shm;
//    lmice_event_t   evt;
//};
//typedef struct lmice_resource_s lmice_resource_t;

//struct lmice_timer_s
//{
//    uint32_t type;                   //ticker timer
//    int32_t state;                  // 状态
//    int32_t size;                   // 触发计数量
//    int32_t tick;                   // 周期长度
//    lmice_event_t   evt;            // 响应事件

//    // 状态量
//    uint64_t count;                 // 已完成触发数量
//    int64_t begin_tick;             // 开始时间
//};
//typedef struct lmice_timer_s lmice_timer_t;

//struct lmice_custom_event_s
//{
//    uint32_t type;
//    uint32_t size;
//    int32_t state;
//    int32_t reserved;
//    uint64_t eids[16];
//    uint64_t id;
//    lmice_event_t evt;
//};
//typedef struct lmice_custom_event_s lmice_custom_event_t;



//struct lmice_client_info_s
//{
//    uint32_t type;
//    uint32_t version;
//    uint32_t size;
//    uint64_t lock;
//    uint64_t eid;
//    uint64_t next_client;

//    lmice_timer_t timer[128];
//    lmice_custom_event_t ce[128];
//    lmice_resource_t res[128];

//};
//typedef struct lmice_client_info_s lmice_client_info_t;

class lmice_spi
{
public:

    // 辅助函数
    int open_shm(uint64_t hval, lmice_shm_t* shm);
    int init();
    //场景管理
    int join_session(uint64_t session_id);
    int leave_session(uint64_t session_id);
    //资源注册
    int register_publish(const char* type, const char* inst, int size, uint64_t *event_id);
    int register_subscribe(const char* type, const char* inst, uint64_t* event_id);

    //事件管理
    int register_tick_event(int period, int size, int due, uint64_t* event_id);
    int register_timer_event(int period, int size, int due, uint64_t* event_id);
    int register_custom_event(uint64_t* event_list, size_t count, uint64_t* event_id);

    //可信计算,QoS管理
    int set_tc_level(int level);
    int set_qos_level(int level);

    //资源回收
    int join();

    int commit();

    void printclients();
    int create_shm_resource(uint64_t id, int size, lm_mesg_res_t *res);
    int create_work_resource(uint64_t id, int size, lm_shm_res_t *res);
    int open_server_resource();
    int create_worker_resource();

public:
    uint64_t    m_session_id;
    uint64_t    m_type_id;
    uint64_t    m_inst_id;
    pid_t       m_process_id;
    pid_t       m_thread_id;

    //    uint64_t    m_server_eid;
    //    lmice_client_info_t *m_client;
    //    lmice_event_t     m_server_evt;

    lm_server_t *m_server;
    lm_worker_t * m_worker;
    lm_shm_res_t   res_server;
    lm_worker_res_t *m_res;
    evtfd_t         m_worker_efd;
};

lmice_shm_t client_shm = {0, DEFAULT_SHM_SIZE*16,0, 0,   CLIENT_SHMNAME};
lmice_shm_t board_shm  = {0, DEFAULT_SHM_SIZE*2, 0,0,  BOARD_SHMNAME};

int lmice_spi::commit()
{
    m_worker->state = WORKER_MODIFIED;
    return eal_event_awake(res_server.efd);
}
int lmice_spi::create_work_resource(uint64_t id, int size, lm_shm_res_t *res)
{
    int ret = 0;
    lmice_event_t   evt;
    lmice_shm_t     shm;
    eal_shm_zero(&shm);
    shm.size = size;
    eal_shm_hash_name(id, shm.name);
    ret = eal_shm_create(&shm);
    if(ret != 0)
    {
        lmice_critical_print("create shm[%s] size(%d) failed[%d]\n", shm.name, shm.size, ret);
        return ret;
    }

    eal_event_zero(&evt);
    eal_event_hash_name(id, evt.name);
    lmice_debug_print("create event[%s]", evt.name);
    ret = eal_event_create(&evt);
    if(ret != 0)
    {
        lmice_critical_print("create event failed[%d]\n", ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    res->addr = shm.addr;
    res->sfd = shm.fd;
    res->efd = evt.fd;

    return ret;
}

int lmice_spi::create_shm_resource(uint64_t id, int size, lm_mesg_res_t *res)
{
    int ret = 0;
    //    lmice_event_t   evt;
    lmice_shm_t     shm;
    eal_shm_zero(&shm);
    shm.size = size;
    eal_shm_hash_name(id, shm.name);
    ret = eal_shm_create(&shm);
    if(ret != 0)
    {
        lmice_critical_print("create shm[%s] size(%d) failed[%d]\n", shm.name, shm.size, ret);
        return ret;
    }

    //    eal_event_zero(&evt);
    //    eal_event_hash_name(id, evt.name);
    //    lmice_debug_print("create event[%s]", evt.name);
    //    ret = eal_event_create(&evt);
    //    if(ret != 0)
    //    {
    //        lmice_critical_print("create event failed[%d]\n", ret);
    //        eal_shm_close(shm.fd, shm.addr);
    //        return ret;
    //    }

    res->addr = shm.addr;
    res->sfd = shm.fd;
    //    res->efd = evt.fd;

    return ret;
}

int lmice_spi::create_worker_resource()
{
    int ret = 0;

    m_inst_id = eal_hash64_fnv1a(&m_process_id, sizeof(m_process_id));
    m_inst_id = eal_hash64_fnv1a(&m_thread_id, sizeof(m_thread_id));
    m_inst_id = eal_hash64_more_fnv1a(CLIENT_SHMNAME, sizeof(CLIENT_SHMNAME)-1, m_inst_id);
    ret = create_work_resource(m_inst_id, DEFAULT_SHM_SIZE*16, &m_res->res);

    m_worker = (lm_worker_t*)m_res->res.addr;
    m_worker->lock = 0;


    return 0;


}

int lmice_spi::open_server_resource()
{
    int ret = 0;
    uint64_t hval = 0;
    lmice_event_t   evt;
    lmice_shm_t     shm;


    /*打开平台公共区域共享内存*/
    eal_shm_zero(&shm);
    hval = eal_hash64_fnv1a(BOARD_SHMNAME, sizeof(BOARD_SHMNAME)-1);
    eal_shm_hash_name(hval, shm.name);
    ret = eal_shm_open_readwrite(&shm);
    if(ret != 0)
    {
        lmice_critical_print("open server shm failed[%d]\n", ret);
        return ret;
    }


    /*打开公共区域管理事件*/
    eal_event_zero(&evt);
    eal_event_hash_name(hval, evt.name);
    ret = eal_event_open(&evt);
    if(ret != 0)
    {
        lmice_critical_print("open server event failed[%d]\n", ret);
        eal_shm_close(shm.fd, shm.addr);
        return ret;
    }

    m_server = (lm_server_t*)shm.addr;
    res_server.addr = shm.addr;
    res_server.efd = evt.fd;
    res_server.sfd = shm.fd;
    return ret;
}

int lmice_spi::open_shm(uint64_t hval, lmice_shm_t* shm)
{
    int ret;

    eal_shm_zero(shm);
    ret = eal_shm_hash_name(hval, shm->name);
    ret = eal_shm_open_readwrite(shm);
    return ret;
}

int lmice_spi::init()
{
    int ret;

    //避免重复初始化
    if( m_worker != 0
            && m_server != 0)
        return 0;

    //默认session_id = 0
    m_session_id = DEFAULT_SESSION_ID;

    //默认type id = 0
    m_type_id = 0;

    //获取当前进程ID
    m_process_id = getpid();

    //线程ID
    m_thread_id = gettid();

    m_res = new lm_worker_res_t;

    //打开平台公共区域共享内存
    ret = open_server_resource();

    //新建客户端共享内存区域
    ret = create_worker_resource();

    //注册客户端信息到平台
    do {
        lm_worker_info_t info={LMICE_VERSION,
                               WORKER_MODIFIED,
                               m_process_id,
                               m_thread_id,
                               m_type_id,
                               m_inst_id};
        //        info.version = LMICE_VERSION;
        //        info.size = DEFAULT_SHM_SIZE;
        //        info.process_id = m_pid;
        //        info.instance_id = m_id;

        ret = eal_spin_lock( &m_server->lock );

        lm_worker_info_t *p;
        int i;
        for(p= &m_server->worker,i=0; i<MAX_CLIENT_COUNT;++p, ++i)
        {
            if(p->inst_id == 0)
            {
                memcpy(p, &info, sizeof(info));
                break;
            }
        }
        eal_spin_unlock(&m_server->lock);

        //唤醒平台管理
        //eal_event_awake(res_server.efd);

    } while(0);

    return 0;
}

//加入场景
int lmice_spi::join_session(uint64_t session_id)
{
    m_session_id = session_id;
    return 0;
}

//退出场景
int lmice_spi::leave_session(uint64_t session_id)
{
    UNREFERENCED_PARAM(session_id);

    m_session_id = DEFAULT_SESSION_ID;
    return 0;
}

/**
 * @brief lmice_spi::register_publish  向平台注册发布资源
 * @param type
 * @param inst
 * @param size
 * @param res
 * @return
 */
int lmice_spi::register_publish(const char *type, const char *inst, int size, uint64_t* event_id)
{
    int ret = 0;
    uint64_t inst_id = 0;
    uint64_t type_id = 0;
    inst_id = eal_hash64_fnv1a(&m_session_id, sizeof(m_session_id));
    inst_id = eal_hash64_more_fnv1a(type, strlen(type), inst_id);
    inst_id = eal_hash64_more_fnv1a(inst, strlen(inst), inst_id);

    type_id = eal_hash64_fnv1a(type, strlen(type));
    //创建资源
    lm_mesg_res_t *res = NULL;
    lm_mesg_info_t *info = NULL;
    size_t i = 0;
    bool find = false;
    /*register*/
    lmice_debug_print("pub try lock %llu\n",  m_worker->lock);
    ret = eal_spin_trylock(&(m_worker->lock));
    if(ret != 0)
    {
        lmice_debug_print("pub lock failed\n");
        return ret;
    }

    for(i=0; i<128; ++i )
    {
        info = m_worker->mesg +i;
        res = m_res->mesg + i;

        if(info->inst_id == 0)
        {
            find = true;
            break;
        }
    }

    eal_spin_unlock(&m_worker->lock);

    lmice_debug_print ("pub check find\n");
    if(!find )
        return 1; //No Found

    ret = create_shm_resource(inst_id, size, res);
    if(ret != 0)
    {
        lmice_critical_print("create shm resource failed\n");
        return ret;
    }

    info->type = PUBLISH_RESOURCE_TYPE;
    info->size = size;
    info->period = 0;
    info->inst_id = inst_id;
    info->type_id = type_id;

    *event_id = inst_id;

    //注册IPC信息,通知平台更新
    lmice_debug_print("pub inst %llu\n", inst_id);

    return 0;

}

int lmice_spi::register_subscribe(const char* type, const char* inst, uint64_t *event_id)
{
    int ret;
    uint64_t inst_id;
    uint64_t type_id;
    inst_id = eal_hash64_fnv1a(&m_session_id, sizeof(m_session_id));
    inst_id = eal_hash64_more_fnv1a(type, strlen(type), inst_id);
    if(inst != NULL)
        inst_id = eal_hash64_more_fnv1a(inst, strlen(inst), inst_id);

    type_id = eal_hash64_fnv1a(type, strlen(type));

    //创建资源
    lm_mesg_res_t *res = NULL;
    lm_mesg_info_t *info = NULL;
    size_t i = 0;
    bool find = false;

    ret = eal_spin_trylock(&m_worker->lock);
    if(ret != 0)
        return ret;

    for(i=0; i<128; ++i )
    {
        info = m_worker->mesg +i;
        res = m_res->mesg + i;

        if(info->inst_id == 0)
        {
            find = true;

            break;
        }
    }

    eal_spin_unlock(&m_worker->lock);

    if(!find )
        return 1; //No Found

    ret = create_shm_resource(inst_id, DEFAULT_SHM_SIZE, res);
    if(ret != 0)
    {
        lmice_critical_print("create shm resource failed\n");
        return ret;
    }

    info->type = SUBSCRIBE_RESOURCE_TYPE;
    info->size = DEFAULT_SHM_SIZE;
    info->period = 0;
    info->inst_id = inst_id;
    info->type_id = type_id;

    *event_id = inst_id;

    //注册IPC信息,通知平台更新
    lmice_debug_print("sub inst %llu\n", inst_id);

    return 0;
}

int lmice_spi::register_tick_event(int period, int size, int due, uint64_t* event_id)
{
    int ret = TICKER_TYPE;
    uint64_t inst_id = 0;
    inst_id = eal_hash64_fnv1a(&m_session_id, sizeof(m_session_id));
    inst_id = eal_hash64_more_fnv1a(&m_inst_id, sizeof(m_inst_id), inst_id);
    inst_id = eal_hash64_more_fnv1a(&ret, sizeof(ret), inst_id);
    inst_id = eal_hash64_more_fnv1a(&due, sizeof(due), inst_id);
    inst_id = eal_hash64_more_fnv1a(&period, sizeof(period), inst_id);
    inst_id = eal_hash64_more_fnv1a(&size, sizeof(size), inst_id);

    //创建资源
    lm_timer_info_t *info = NULL;
    lm_timer_res_t  *res  = NULL;
    size_t i = 0;
    bool find = false;

    ret = eal_spin_trylock(&m_worker->lock);
    if(ret != 0)
        return ret;

    for(i=0; i<128; ++i )
    {
        info = m_worker->timer +i;
        res = m_res->timer + i;

        if(info->inst_id == 0)
        {
            find = true;

            break;
        }
    }

    eal_spin_unlock(&m_worker->lock);

    if(!find )
        return 1; //No Found


    //    //资源事件
    //    lmice_event_t evt;
    //    eal_event_zero(&evt);
    //    eal_event_hash_name(hval, evt.name);
    //    ret = eal_event_create(&evt);
    //    if(ret != 0)
    //    {
    //        lmice_critical_print("create ticker event(%d[%d] - %d) failed(%d)\n", period, size, due, ret);
    //        return ret;
    //    }

    //    res->efd = evt.fd;

    info->type = TICKER_TYPE;
    info->size = size;
    info->period = period;
    info->due = due;
    info->inst_id = inst_id;
    info->timer.count = 0;
    info->timer.begin = 0;
    info->timer.state.value[0] = 0x0;

    *event_id = inst_id;

    //注册IPC信息,通知平台更新
    return 0;
}

int lmice_spi::register_timer_event(int period, int size, int due, uint64_t* event_id)
{
    int ret = TIMER_TYPE;
    uint64_t inst_id;
    inst_id = eal_hash64_fnv1a(&m_session_id, sizeof(m_session_id));
    inst_id = eal_hash64_more_fnv1a(&m_process_id, sizeof(m_process_id), inst_id);
    inst_id = eal_hash64_more_fnv1a(&ret, sizeof(ret), inst_id);
    inst_id = eal_hash64_more_fnv1a(&due, sizeof(due), inst_id);
    inst_id = eal_hash64_more_fnv1a(&period, sizeof(period), inst_id);
    inst_id = eal_hash64_more_fnv1a(&size, sizeof(size), inst_id);

    //创建资源
    lm_timer_info_t *info = NULL;
    lm_timer_res_t  *res  = NULL;
    size_t i = 0;
    bool find = false;

    ret = eal_spin_trylock(&m_worker->lock);
    if(ret != 0)
        return ret;

    for(i=0; i<128; ++i )
    {
        info = m_worker->timer +i;
        res = m_res->timer + i;

        if(info->inst_id == 0)
        {
            find = true;

            break;
        }
    }

    eal_spin_unlock(&m_worker->lock);

    if(!find )
        return 1; //No Found


    //    //资源事件
    //    lmice_event_t evt;
    //    eal_event_zero(&evt);
    //    eal_event_hash_name(inst_id, evt.name);
    //    ret = eal_event_create(&evt);
    //    if(ret != 0)
    //    {
    //        lmice_critical_print("create timer event(%d[%d] - %d) failed(%d)\n", period, size, due, ret);
    //        return ret;
    //    }

    //    res->efd = evt.fd;

    info->type = TIMER_TYPE;
    info->size = size;
    info->period = period;
    info->due = due;
    info->inst_id = inst_id;
    info->timer.count = 0;
    info->timer.begin = 0;

    *event_id = inst_id;

    return 0;
}

int lmice_spi::register_custom_event(uint64_t* event_list, size_t size, uint64_t *event_id)
{
    int ret = CUSTOM_TYPE;
    uint64_t inst_id;
    size_t sz;

    inst_id = eal_hash64_fnv1a(&m_session_id, sizeof(m_session_id));
    inst_id = eal_hash64_more_fnv1a(&m_process_id, sizeof(m_process_id), inst_id);
    inst_id = eal_hash64_more_fnv1a(&ret, sizeof(ret), inst_id);
    inst_id = eal_hash64_more_fnv1a(&size, sizeof(size), inst_id);
    for(sz = 0; sz < size; ++sz)
    {
        inst_id = eal_hash64_more_fnv1a((event_list+sz), sizeof(uint64_t), inst_id);
    }

    //创建资源
    lm_action_info_t *info = NULL;
    lm_action_res_t  *res  = NULL;
    size_t i = 0;
    bool find = false;

    ret = eal_spin_trylock(&m_worker->lock);
    if(ret != 0)
        return ret;

    for(i=0; i<128; ++i )
    {
        info = m_worker->action +i;
        res = m_res->action + i;

        if(info->inst_id == 0)
        {
            find = true;

            break;
        }
    }

    eal_spin_unlock(&m_worker->lock);

    if(!find )
        return 1; //No Found

    //    //资源事件
    //    lmice_event_t evt;
    //    eal_event_zero(&evt);
    //    eal_event_hash_name(hval, evt.name);
    //    ret = eal_event_create(&evt);
    //    if(ret != 0)
    //    {
    //        lmice_critical_print("create action event failed(%d)\n", ret);
    //        return ret;
    //    }

    //    res->efd = evt.fd;
    info->type = CUSTOM_EVENT_TYPE;
    info->size = size;
    info->inst_id = inst_id;
    for(i=0; i< size; ++i)
        info->act_ids[i] = *(event_list+i);
    memset(&info->state, 0, sizeof(info->state));


    *event_id = inst_id;

    //注册IPC信息,通知平台更新

    return 0;
}

int lmice_spi::set_tc_level(int level)
{
    return 0;
}

int lmice_spi::set_qos_level(int level)
{
    return 0;
}

#include <vector>
int lmice_spi::join()
{
    evtfd_t evts = NULL;
    if(m_res->res.efd != 0)
    {
        evts = m_res->res.efd;
    }

    std::vector<lm_timer_info_t*> tlist;
    size_t i=0;
    lm_timer_info_t * info;

    for(;;)
    {
        DWORD hr = WaitForSingleObject(evts,
                                       INFINITE);
        switch(hr)
        {
        case WAIT_OBJECT_0:
            for(i=0; i< 128; ++i)
            {
                info = m_worker->timer +i;
                if(info->inst_id == 0)
                    continue;
                if( info->timer.state.value[0] == 0xFF )
                {
                    info->timer.state.value[0] = 0x0;
                    lmice_debug_print("server send timer[%lu] event [%lld] \n",i, info->period);
                }
            }

            break;
        case WAIT_TIMEOUT:
            return 1;
            break;
        case WAIT_FAILED:
        {
            DWORD err =GetLastError();
            lmice_debug_print("event wait failed (%llu)\n", err);
            return 1;
            break;
        }
        default:
            lmice_debug_print("event fired (%llu)\n", hr - WAIT_OBJECT_0);
            return 0;
            break;
        }
    }


    return 0;
}

void lmice_spi::printclients()
{
    lm_worker_info_t *inst;
    int i;
    int pos = 0;

    int64_t begin = m_server->tm.system_time;
    SYSTEMTIME st;
    FileTimeToSystemTime((FILETIME*)&begin, &st);

    printf("%llu\n%d-%d-%d %d:%d:%d %d\n",
           begin,
           st.wYear, st.wMonth, st.wDay,
           st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    for(inst = &m_server->worker, i=0;i<MAX_CLIENT_COUNT; ++inst, ++i)
    {
        if(inst->inst_id != 0)
        {
            ++pos;
            lmice_debug_print("inst[ %d ] version %d, processid =%llu, threadid= %llu, instid=%llX",
                              pos, inst->version,
                              inst->process_id,
                              inst->thread_id,
                              inst->inst_id);
        }
    }
    int64_t end = m_server->tm.system_time;
    printf("total time %x\n", end - begin);
}


/*****************************************************/
/* Implementation */

//建立中间件实例
Thread lmice_spi spi;

int main(int argc, char* argv[])
{

    UNREFERENCED_PARAM(argc);
    UNREFERENCED_PARAM(argv);

    int ret;

    //    printf("sizeof(short)     = %d\n", sizeof(short));
    //            printf("sizeof(int)       = %d\n", sizeof(int));
    //            printf("sizeof(long)      = %d\n", sizeof(long));
    //            printf("sizeof(long long) = %d\n\n", sizeof(long long));

    //            printf("sizeof(size_t)    = %d\n", sizeof(size_t));
    //            printf("sizeof(off_t)     = %d\n", sizeof(off_t));
    //            printf("sizeof(void *)    = %d\n", sizeof(void *));
    //    return 1;

//中间件初始化
ret = spi.init();
if(ret !=0)
{
    lmice_critical_print("init failed[%d]", ret);
    return 1;
}


    if(argc > 1)
    {
        spi.printclients();
        return 0;
    }


//加入会话
ret = spi.join_session(1);
if(ret != 0)
    return ret;

    lmice_critical_print("begin publish");
//注册资源发布与订阅
uint64_t pe[3], se;
spi.register_publish("my message 1", "spider-1", 32, &pe[0]);
spi.register_publish("my message 2", "spider-1", 32, &pe[1]);
spi.register_publish("another message 3", "spider-2", 32, &pe[2]);

//注册资源订阅
spi.register_subscribe("remote message ", ALL_INSTANCE, &se);

//注册仿真时间定时器
uint64_t te[2];
spi.register_tick_event(300000, 0, 0, &te[0]);

//注册系统定时器
lmice_critical_print("begin timer");
spi.register_timer_event(5000000, 2, 0, &te[1]);

//注册事件状态机
uint64_t evts[2], cevt;
evts[0] = te[0]; evts[1] = te[1];
spi.register_custom_event(evts, 2, &cevt);

//可信计算与QoS设置
spi.set_qos_level(NO_QOS_SUPPORT);
spi.set_tc_level(NO_TRUST_COMPUTION_SUPPORT);

//通知平台工作
spi.commit();

//阻塞当前线程,回收资源
spi.join();

//...运行结束...

//清理中间件

    //清理中间件
    lmice_critical_print("begin join");


    return 0;

}
