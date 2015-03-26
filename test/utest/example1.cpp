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
    TICKER_TYPE = 1,
    TIMER_TYPE = 2,
    CUSTOM_TYPE = 3,
    INFINITY = -1,

    PUBLISH_RESOURCE_TYPE = 1,
    SUBSCRIBE_RESOURCE_TYPE = 2,
    TIMER_EVENT_TYPE = 3,
    TICKER_EVENT_TYPE = 4,
    CUSTOM_EVENT_TYPE = 5,
    QOS_CONTROL_TYPE = 6,
    TRUST_CONTROL_TYPE
};


struct lmice_resource_s
{
    uint32_t        type;   // publish subscribe
    int32_t         state;  // 0 freed 1 modify 2 work
    int64_t         time;   // state lock time
    lmice_shm_t     shm;
    lmice_event_t   evt;
};
typedef struct lmice_resource_s lmice_resource_t;

struct lmice_timer_s
{
    uint32_t type;                   //ticker timer
    int32_t state;                  // 状态
    int32_t size;                   // 触发计数量
    int32_t tick;                   // 周期长度
    lmice_event_t   evt;            // 响应事件

    // 状态量
    uint64_t count;                 // 已完成触发数量
    int64_t begin_tick;             // 开始时间
};
typedef struct lmice_timer_s lmice_timer_t;

struct lmice_custom_event_s
{
    uint32_t type;
    uint32_t size;
    int32_t state;
    int32_t reserved;
    uint64_t eids[16];
    uint64_t id;
    lmice_event_t evt;
};
typedef struct lmice_custom_event_s lmice_custom_event_t;



struct lmice_client_info_s
{
    uint32_t type;
    uint32_t version;
    uint32_t size;
    uint64_t lock;
    uint64_t eid;
    uint64_t next_client;

    lmice_timer_t timer[128];
    lmice_custom_event_t ce[128];
    lmice_resource_t res[128];

};
typedef struct lmice_client_info_s lmice_client_info_t;

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
    int register_tick_event(int period, int size, int begin, uint64_t* event_id);
    int register_timer_event(int period, int size, int begin, uint64_t* event_id);
    int register_custom_event(uint64_t* event_list, size_t count, uint64_t* event_id);

    //可信计算,QoS管理
    int set_tc_level(int level);
    int set_qos_level(int level);

    //资源回收
    int join();

    void printclients();

public:
    uint64_t    m_sid;
    uint64_t    m_tid;
    uint64_t    m_iid;
    uint64_t    m_pid;
    uint64_t    m_server_eid;
    lmice_client_info_t *m_client;
    lm_server_info_t *m_server;
    lmice_event_t     m_server_evt;
};

lmice_shm_t client_shm = {0, DEFAULT_SHM_SIZE*16,0, 0,   CLIENT_SHMNAME};
lmice_shm_t board_shm  = {0, DEFAULT_SHM_SIZE*2, 0,0,  BOARD_SHMNAME};

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
    uint64_t hval;

    //避免重复初始化
    if(m_pid != 0
            && m_client != 0
            && m_server != 0)
        return 0;

    //默认session_id = 0
    m_sid = 0;

    //默认type id = 0
    m_tid = 0;

    //获取当前进程ID
    m_pid = getpid();

    //打开平台公共区域共享内存
    hval = eal_hash64_fnv1a(BOARD_SHMNAME, sizeof(BOARD_SHMNAME)-1);
    eal_shm_hash_name(hval, board_shm.name);
    ret = eal_shm_open_readwrite(&board_shm);
    if(ret != 0)
    {
        lmice_critical_print("open board_shm failed\n");
        return 1;
    }
    m_server = (lm_server_info_t*)((void*)(board_shm.addr));

    //新建客户端共享内存区域
    m_iid = eal_hash64_fnv1a(&m_pid, sizeof(m_pid));
    m_iid = eal_hash64_more_fnv1a(CLIENT_SHMNAME, sizeof(CLIENT_SHMNAME)-1, m_iid);
    eal_shm_hash_name(m_iid, client_shm.name);
    ret = eal_shm_create(&client_shm);
    if(ret != 0)
    {
        lmice_critical_print("create client_shm[%s] size(%d) failed\n", client_shm.name, client_shm.size);
        eal_shm_destroy(&board_shm);
        return 1;
    }
    lmice_debug_print("client shm size %llu", client_shm.size);



    m_client = (lmice_client_info_t*)((void*)client_shm.addr);

    lmice_event_t ec;
    eal_event_zero(&ec);
    eal_event_hash_name(hval, ec.name);
    lmice_debug_print("create client event[%s]", ec.name);
    eal_event_create(&ec);
    m_client->eid = (uint64_t)ec.fd;
    lmice_debug_print("create client event [%ull]", m_client->eid);

    lmice_event_t m_server_evt;
    eal_event_zero(&m_server_evt);
    eal_event_hash_name(m_server->event_id, m_server_evt.name);
    ret = eal_event_open(&m_server_evt);
    if(ret != 0)
    {
        lmice_critical_print("open server event failed[%d]", ret);
        return 1;
    }
    //注册客户端信息到平台
    do {
        lm_instance_info_t info={LMICE_VERSION,
                                    0,
                                    m_pid,
                                    pthread_self(),
                                    m_tid,
                                    m_iid};
        //        info.version = LMICE_VERSION;
        //        info.size = DEFAULT_SHM_SIZE;
        //        info.process_id = m_pid;
        //        info.instance_id = m_id;

        ret = eal_spin_lock( &m_server->lock );

        lm_instance_info_t *p;
        int i;
        for(p= &m_server->inst,i=0; i<MAX_CLIENT_COUNT;++p, ++i)
        {
            if(p->instance_id == 0)
            {
                memcpy(p, &info, sizeof(info));
                break;
            }
        }
        eal_spin_unlock(&m_server->lock);

        //唤醒平台管理
        eal_event_awake((uint64_t)m_server_evt.fd);

    } while(0);

    return 0;
}

//加入场景
int lmice_spi::join_session(uint64_t session_id)
{
    m_sid = session_id;
    return 0;
}

//退出场景
int lmice_spi::leave_session(uint64_t session_id)
{
    m_sid = DEFAULT_SESSION_ID;
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
    int ret;
    uint64_t hval;
    hval = eal_hash64_fnv1a(&m_sid, sizeof(m_sid));
    hval = eal_hash64_more_fnv1a(type, sizeof(type)-1, hval);
    hval = eal_hash64_more_fnv1a(inst, sizeof(inst)-1, hval);

    //创建资源
    lmice_resource_t* res;
    int i;


    ret = eal_spin_trylock(&m_client->lock);
    if(ret != 0)
        return ret;

    for(res = m_client->res, i=0; i<128; ++res, ++i )
    {
        if(res->state == FREE_RESOURCE)
        {
            res->state = MODIFY_RESOURCE;
            res->type = PUBLISH_TYPE;
            i=-1;
            break;
        }
    }

    eal_spin_unlock(&m_client->lock);
    if(i != -1)
        return 1; //No Found

    //共享内存
    eal_shm_zero(&res->shm);
    res->shm.size = size;
    eal_shm_hash_name(hval, res->shm.name);
    ret = eal_shm_create(&res->shm);
    if(ret != 0)
    {
        res->state = FREE_RESOURCE;
        lmice_critical_print("create publish resource(%s - %s) failed(%d)\n", inst, type, ret);
        return ret;
    }
    //资源事件
    eal_event_zero(&res->evt);
    eal_event_hash_name(hval, res->evt.name);
    ret = eal_event_create(&res->evt);
    if(ret != 0)
    {
        eal_shm_destroy(&res->shm);
        res->state = FREE_RESOURCE;
        lmice_critical_print("create publish resource event(%s - %s) failed(%d)\n", inst, type, ret);
        return ret;
    }
    *event_id = (uint64_t)res->evt.fd;
    //注册IPC信息,通知平台更新
    res->state = WORK_RESOURCE;
    eal_event_awake((uint64_t)m_server_evt.fd);

    return 0;

}

int lmice_spi::register_subscribe(const char* type, const char* inst, uint64_t *event_id)
{
    int ret;
    uint64_t hval;
    hval = eal_hash64_fnv1a(&m_sid, sizeof(m_sid));
    hval = eal_hash64_more_fnv1a(type, sizeof(type)-1, hval);
    if(inst != 0)
        hval = eal_hash64_more_fnv1a(inst, sizeof(inst)-1, hval);

    //创建资源
    lmice_resource_t* res;
    int i;


    ret = eal_spin_trylock(&m_client->lock);
    if(ret != 0)
        return ret;

    for(res = m_client->res, i=0; i<128; ++res, ++i )
    {
        if(res->state == FREE_RESOURCE)
        {
            res->state = MODIFY_RESOURCE;
            res->type = SUBSCRIBE_TYPE;
            i=-1;
            break;
        }
    }

    eal_spin_unlock(&m_client->lock);
    if(i != -1)
        return 1; //No Found

    //共享内存
    eal_shm_zero(&res->shm);
    res->shm.size = 0;
    eal_shm_hash_name(hval, res->shm.name);

    //资源事件
    eal_event_zero(&res->evt);
    eal_event_hash_name(hval, res->evt.name);
    ret = eal_event_create(&res->evt);
    if(ret != 0)
    {
        res->state = FREE_RESOURCE;
        lmice_critical_print("create publish resource event(%s - %s) failed(%d)\n", inst, type, ret);
        return ret;
    }
    *event_id = (uint64_t)res->evt.fd;
    //注册IPC信息,通知平台更新
    res->state = WORK_RESOURCE;
    eal_event_awake((uint64_t)m_server_evt.fd);

    return 0;
}

int lmice_spi::register_tick_event(int period, int size, int begin, uint64_t* event_id)
{
    int ret = TICKER_TYPE;
    uint64_t hval;
    hval = eal_hash64_fnv1a(&m_sid, sizeof(m_sid));
    hval = eal_hash64_more_fnv1a(&m_pid, sizeof(m_pid), hval);
    hval = eal_hash64_more_fnv1a(&ret, sizeof(ret), hval);
    hval = eal_hash64_more_fnv1a(&period, sizeof(period), hval);
    hval = eal_hash64_more_fnv1a(&begin, sizeof(begin), hval);
    hval = eal_hash64_more_fnv1a(&size, sizeof(size), hval);

    //创建资源
    lmice_timer_t* timer;
    int i;


    ret = eal_spin_trylock(&m_client->lock);
    if(ret != 0)
        return ret;

    for(timer = m_client->timer, i=0; i<128; ++timer, ++i )
    {
        if(timer->state == FREE_RESOURCE)
        {
            timer->state = MODIFY_RESOURCE;
            i=-1;
            break; //Found
        }
    }

    eal_spin_unlock(&m_client->lock);
    if(i != -1)
        return 1; //No Found

    timer->type = TICKER_TYPE;
    timer->begin_tick = begin;
    timer->count = 0;
    timer->size = size;
    timer->tick = period;

    //资源事件
    eal_event_zero(&timer->evt);
    eal_event_hash_name(hval, timer->evt.name);
    ret = eal_event_create(&timer->evt);
    if(ret != 0)
    {
        timer->state = FREE_RESOURCE;
        lmice_critical_print("create ticker event(%d[%d] - %d) failed(%d)\n", period, size, begin, ret);
        return ret;
    }

    *event_id =(uint64_t) timer->evt.fd;

    //注册IPC信息,通知平台更新
    timer->state = WORK_RESOURCE;
    eal_event_awake((uint64_t)m_server_evt.fd);
    return 0;
}

int lmice_spi::register_timer_event(int period, int size, int begin, uint64_t* event_id)
{
    int ret = TIMER_TYPE;
    uint64_t hval;
    hval = eal_hash64_fnv1a(&m_sid, sizeof(m_sid));
    hval = eal_hash64_more_fnv1a(&m_pid, sizeof(m_pid), hval);
    hval = eal_hash64_more_fnv1a(&ret, sizeof(ret), hval);
    hval = eal_hash64_more_fnv1a(&period, sizeof(period), hval);
    hval = eal_hash64_more_fnv1a(&begin, sizeof(begin), hval);
    hval = eal_hash64_more_fnv1a(&size, sizeof(size), hval);

    //创建资源
    lmice_timer_t* timer;
    int i;


    ret = eal_spin_trylock(&m_client->lock);
    if(ret != 0)
        return ret;

    for(timer = m_client->timer, i=0; i<128; ++timer, ++i )
    {
        if(timer->state == FREE_RESOURCE)
        {
            timer->state = MODIFY_RESOURCE;
            i=-1;
            break; //Found
        }
    }

    eal_spin_unlock(&m_client->lock);
    if(i != -1)
        return 1; //No Found

    timer->type = TIMER_TYPE;
    timer->begin_tick = begin;
    timer->count = 0;
    timer->size = size;
    timer->tick = period;

    //资源事件
    eal_event_zero(&timer->evt);
    eal_event_hash_name(hval, timer->evt.name);
    ret = eal_event_create(&timer->evt);
    if(ret != 0)
    {
        timer->state = FREE_RESOURCE;
        lmice_critical_print("create ticker event(%d[%d] - %d) failed(%d)\n", period, size, begin, ret);
        return ret;
    }

    *event_id =(uint64_t) timer->evt.fd;

    //注册IPC信息,通知平台更新
    timer->state = WORK_RESOURCE;
    eal_event_awake((uint64_t)m_server_evt.fd);
}

int lmice_spi::register_custom_event(uint64_t* event_list, size_t size, uint64_t *event_id)
{
    int ret = CUSTOM_TYPE;
    uint64_t hval;
    size_t sz;
    int i;
    hval = eal_hash64_fnv1a(&m_sid, sizeof(m_sid));
    hval = eal_hash64_more_fnv1a(&m_pid, sizeof(m_pid), hval);
    hval = eal_hash64_more_fnv1a(&ret, sizeof(ret), hval);
    hval = eal_hash64_more_fnv1a(&size, sizeof(size), hval);
    for(sz = 0; sz < size; ++sz)
    {
        hval = eal_hash64_more_fnv1a((event_list+sz), sizeof(uint64_t), hval);
    }

    lmice_custom_event_t *ev;

    ret = eal_spin_trylock(&m_client->lock);
    if(ret != 0)
        return ret;

    for(ev = m_client->ce, i=0; i<128; ++ev, ++i )
    {
        if(ev->state == FREE_RESOURCE)
        {
            ev->state = MODIFY_RESOURCE;
            i=-1;
            break; //Found
        }
    }

    eal_spin_unlock(&m_client->lock);
    if(i != -1)
        return 1; //No Found

    ev->size = size;
    for(sz = 0; sz < size; ++sz)
    {
        ev->eids[sz] = *(event_list+sz);
    }

    //资源事件
    lmice_event_t &evt = ev->evt;
    eal_event_zero(&evt);
    eal_event_hash_name(hval, evt.name);
    ret = eal_event_create(&evt);
    if(ret != 0)
    {
        ev->state = FREE_RESOURCE;
        lmice_critical_print("create custom event(%ld[%ld]) failed(%d)\n", *event_list, size, ret);
        return ret;
    }
    *event_id = (uint64_t)evt.fd;
    ev->id = *event_id;
    //注册IPC信息,通知平台更新
    ev->state = WORK_RESOURCE;
    eal_event_awake((uint64_t)m_server_evt.fd);

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
    std::vector<HANDLE> evts;
    int i;
    lmice_timer_t* timer;
    lmice_custom_event_t *ev;
    lmice_resource_t *res;


    if(m_client->eid != 0)
    {
        evts.push_back((HANDLE)m_client->eid);
    }

    for(res = m_client->res, i=0; i<128; ++res, ++i )
    {
        if(res->state == WORK_RESOURCE)
        {
            evts.push_back((HANDLE)res->evt.fd);
        }
    }



    for(timer = m_client->timer, i=0; i<128; ++timer, ++i )
    {
        if(timer->state == WORK_RESOURCE)
        {
            evts.push_back((HANDLE)timer->evt.fd);
        }
    }

    for(ev = m_client->ce, i=0; i<128; ++ev, ++i )
    {
        if(ev->state == WORK_RESOURCE)
        {
            evts.push_back((HANDLE)ev->evt.fd);
        }
    }

    lmice_debug_print("total events=%d, %llu", evts.size(), m_client->eid);
    const HANDLE* ph = &evts[0];
    for(;;)
    {
        DWORD hr = WaitForMultipleObjects( evts.size(),
                                           ph,
                                           FALSE,
                                           INFINITE);
        switch(hr)
        {
        case WAIT_OBJECT_0:
            return 0;
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
    lm_instance_info_t *inst;
    int i;
    int pos = 0;
    for(i=0; i<2;++i)
    {
        Sleep(1);
        int64_t tm = m_server->tm.system_time;
        SYSTEMTIME st;
        FileTimeToSystemTime((FILETIME*)&tm, &st);

        printf("%llu\n%d-%d-%d %d:%d:%d %d\n",
               tm,
               st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }
    for(inst = &m_server->inst, i=0;i<MAX_CLIENT_COUNT; ++inst, ++i)
    {
        if(inst->instance_id != 0)
        {
            ++pos;
            lmice_debug_print("inst[ %d ] version %d, processid =%llu, threadid= %llu, instid=%llX",
                              pos, inst->version,
                              inst->process_id,
                              inst->thread_id,
                              inst->instance_id);
        }
    }
}


/*****************************************************/
/* Implementation */

class beatheart_reporter :public lmice_event_callback
{
public:
    ~beatheart_reporter()
    {

    }

    int operator() (void* pdata)
    {
        return 0;
    }
};

//建立中间件实例
Thread lmice_spi spi;

int main(int argc, char* argv[])
{
    int ret;

    //    printf("sizeof(short)     = %d\n", sizeof(short));
    //            printf("sizeof(int)       = %d\n", sizeof(int));
    //            printf("sizeof(long)      = %d\n", sizeof(long));
    //            printf("sizeof(long long) = %d\n\n", sizeof(long long));

    //            printf("sizeof(size_t)    = %d\n", sizeof(size_t));
    //            printf("sizeof(off_t)     = %d\n", sizeof(off_t));
    //            printf("sizeof(void *)    = %d\n", sizeof(void *));
    //    return 1;

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
    printf("%ld\n", sizeof(lmice_client_info_t));
    //return 1;

    ret = spi.join_session(1);
    if(ret != 0)
        return ret;

    lmice_critical_print("begin publish");
    //注册资源发布与订阅
    uint64_t pe, se;
    spi.register_publish("beatheart", "172.26.4.153", 32, &pe);
    lmice_critical_print("begin subscribe");
    spi.register_subscribe("beatheart", ALL_INSTANCE, &se);
    lmice_critical_print("created publish subscribe");
    //注册仿真时间定时器
    uint64_t etick, ttick;
    lmice_critical_print("begin ticker");
    spi.register_tick_event(30, INFINITY, TICK_NOW, &etick);

    //注册系统定时器
    lmice_critical_print("begin timer");
    spi.register_timer_event(30, INFINITY, TIMER_NOW, &ttick);

    //注册事件状态机
    uint64_t evts[2], cevt;
    evts[0] = etick; evts[1] = ttick;
    lmice_critical_print("begin state machine");
    spi.register_custom_event(evts, 2, &cevt);

    //可信计算与QoS设置
    lmice_critical_print("begin trust compute");
    spi.set_qos_level(NO_QOS_SUPPORT);
    spi.set_tc_level(NO_TRUST_COMPUTION_SUPPORT);

    //用户工作
    //...

    //清理中间件
    lmice_critical_print("begin join");
    spi.join();

    return 0;

}
