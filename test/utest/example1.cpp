#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

class lmice_resource
{
public:
    void* operator() () const
    {
        return m_ptr;
    }
private:
    void* m_ptr;
    int size;
};

class lmice_event_callback
{
public:
    virtual ~lmice_event_callback() {}
    virtual int operator() (void* pdata) = 0;
};

#define ALL_INSTANCE (const char*)0
enum lmice_common
{
    TICK_NOW = 0,
    TIMER_NOW = 0,
    NO_TRUST_COMPUTION_SUPPORT = 0,
    NO_QOS_SUPPORT=0,
    DEFAULT_SESSION_ID = 0
};

struct lmice_instance_info_s
{
    uint32_t version;
    uint32_t size;

    uint64_t process_id;    //system process id
    uint64_t instance_id;   //client or server instance identity
};

typedef struct lmice_instance_info_s lmice_instance_info_t;


struct lmice_server_info_s
{
    uint64_t lock;
    uint64_t eid;
    lmice_instance_info_t inst;
};

typedef struct lmice_server_info_s lmice_server_info_t;


class lmice_spi
{
  public:
    lmice_spi();
    //场景管理
    int join_session(uint64_t session_id);
    int leave_session(uint64_t session_id);
    //资源注册
    int register_publish(const char* type, const char* inst, int size, lmice_resource* res);
    int register_subscribe(const char* type, const char* inst, lmice_resource* res);

    //事件管理
    int register_tick_event(int period, int begin, uint64_t* event_id, void* pdata, lmice_event_callback* cb);
    int register_timer_event(int period, int begin, uint64_t* event_id, void* pdata, lmice_event_callback* cb);
    int register_custom_event(uint64_t* event_list, size_t count, uint64_t* event_id, void* pdata, lmice_event_callback* cb);

    //可信计算,QoS管理
    int set_tc_level(int level);
    int set_qos_level(int level);

    //资源回收
    int join();

private:
    uint64_t    m_sid;
    uint64_t    m_iid;
    uint64_t    m_pid;
    uint64_t    m_server_eid;
};

#include "eal/lmice_eal_shm.h"
#include "eal/lmice_eal_event.h"
#include "eal/lmice_trace.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_spinlock.h"

#define CLIENT_SHMNAME "CC597303-0F85-40B2-8BDC-4724BD" /** C87B4E */
#define BOARD_SHMNAME "82E0EE49-382C-40E7-AEA2-495999" /** 392D29 */
#define DEFAULT_SHM_SIZE 4096 /** 4KB */
#define MAX_CLIENT_COUNT 200
#define LMICE_VERSION 1


lmice_shm_t client_shm = {0, DEFAULT_SHM_SIZE, 0,   CLIENT_SHMNAME};
lmice_shm_t board_shm  = {0, DEFAULT_SHM_SIZE, 0,  BOARD_SHMNAME};

lmice_spi::lmice_spi()
    :m_sid(0)
{
    int ret;
    lmice_server_info_t* server;

    //获取当前进程ID
    m_pid = getpid();

    //打开平台公共区域共享内存
    ret = eal_shm_open_readwrite(&board_shm);
    if(ret != 0)
    {
        lmice_critical_print("open board_shm failed\n");
        return;
    }
    server = (lmice_server_info_t*)((void*)(board_shm.addr));

    //新建客户端共享内存区域
    m_iid = eal_hash64_fnv1a(&m_pid, sizeof(m_pid));
    m_iid = eal_hash64_more_fnv1a(CLIENT_SHMNAME, sizeof(CLIENT_SHMNAME)-1, m_iid);
    eal_shm_hash_name(m_iid, client_shm.name);
    ret = eal_shm_create(&client_shm);
    if(ret != 0)
    {
        lmice_critical_print("open client_shm failed\n");
        eal_shm_destroy(&board_shm);
        return;
    }

    //注册客户端信息到平台
    do {
        lmice_instance_info_t info={LMICE_VERSION,
                                    DEFAULT_SHM_SIZE,
                                    m_pid,
                                    m_iid};
        //        info.version = LMICE_VERSION;
        //        info.size = DEFAULT_SHM_SIZE;
        //        info.process_id = m_pid;
        //        info.instance_id = m_id;

        ret = eal_spin_lock( &server->lock );

        lmice_instance_info_t *p;
        int i;
        for(p= &server->inst,i=0; i<MAX_CLIENT_COUNT;++p, ++i)
        {
            if(p->process_id == 0)
            {
                memcpy(p, &info, sizeof(info));
                break;
            }
        }
        eal_spin_unlock(&server->lock);

        //唤醒平台管理
        eal_event_awake(server->eid);

    } while(0);

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
int lmice_spi::register_publish(const char *type, const char *inst, int size, lmice_resource *res)
{
    int ret;
    uint64_t hval;
    hval = eal_hash64_fnv1a(&m_sid, sizeof(m_sid));
    hval = eal_hash64_more_fnv1a(type, sizeof(type)-1, hval);
    hval = eal_hash64_more_fnv1a(inst, sizeof(inst)-1, hval);

    //创建资源
    lmice_shm_t shm;
    eal_shm_zero(&shm);
    shm.size = size;
    eal_shm_hash_name(hval, shm.name);
    ret = eal_shm_create(&shm);
    if(ret != 0)
    {
        lmice_critical_print("create publish resource(%s - %s) failed(%d)\n", inst, type, ret);
        return ret;
    }
    //创建资源事件
    eal_event_create();
    //IPC调用

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

int main()
{
    int ret;
    //建立中间件实例
    lmice_spi spi;

    ret = spi.join_session(1);
    if(ret != 0)
        return ret;

    //注册资源发布与订阅
    lmice_resource beatheart;
    lmice_resource allbeat;
    spi.register_publish("beatheart", "172.26.4.153", 32, &beatheart);
    spi.register_subscribe("beatheart", ALL_INSTANCE, &allbeat);

    //注册时间事件
    beatheart_reporter bh;
    uint64_t etick, ttick;
    spi.register_tick_event(30, TICK_NOW, &etick, 0, &bh);
    spi.register_timer_event(30, TIMER_NOW, &ttick, 0, &bh);

    //注册用户事件
    uint64_t evts[2], cevt;
    evts[0] = etick; evts[1] = ttick;
    spi.register_custom_event(evts, 2, &cevt, 0, &bh);

    //可信计算与QoS设置
    spi.set_qos_level(NO_QOS_SUPPORT);
    spi.set_tc_level(NO_TRUST_COMPUTION_SUPPORT);

    //用户工作
    //...

    //清理中间件
    spi.join();

    return 0;

}
