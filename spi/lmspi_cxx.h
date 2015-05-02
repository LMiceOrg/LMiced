#ifndef LMSPI_CXX_H
#define LMSPI_CXX_H

#if defined(LMSPI_PROJECT)
#define dllclass __declspec(dllexport)
#else
#define dllclass __declspec(dllimport)
#endif

#include <stdint.h>

typedef void (* lmice_event_callback)(uint64_t id);

//接口对象类
class dllclass LMspi
{
public:
    virtual ~LMspi() {}
    // 辅助函数
    virtual int init() =0;
    virtual int commit() =0;

    //场景管理
    virtual int join_session(uint64_t session_id) =0;
    virtual int leave_session(uint64_t session_id) =0;
    //资源注册
    virtual int register_publish(const char* type, const char* inst, int size, uint64_t *event_id) =0;
    virtual int register_subscribe(const char* type, const char* inst, uint64_t* event_id) =0;
    //事件管理
    virtual int register_tick_event(int period, int size, int due, uint64_t* event_id) =0;
    virtual int register_timer_event(int period, int size, int due, uint64_t* event_id) =0;
    virtual int register_custom_event(uint64_t* event_list, size_t count, uint64_t* event_id) =0;

    //基于ID的回调函数管理
    virtual int register_callback(uint64_t id, lmice_event_callback *callback) =0;
    virtual int unregister_callback(uint64_t id, lmice_event_callback *callback) =0;

    //可信计算,QoS管理
    virtual int set_tc_level(int level) =0;
    virtual int set_qos_level(int level) =0;

    //阻塞运行与资源回收
    virtual int join() =0;
};

//接口工厂类
class dllclass LMFactory
{
public:
    LMspi* create_spi();
    void delete_spi(LMspi* spi);
};


#endif /** LMSPI_CXX_H */

