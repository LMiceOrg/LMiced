#ifndef LMSPI_IMPL_H
#define LMSPI_IMPL_H
#include <stdint.h>
#include <map>
#include <string>
#include <boost/python.hpp>
#include "lmspi_cxx.h"


extern "C"
{
#include "../eal/lmice_eal_common.h"
#include "../eal/lmice_eal_hash.h"
#include "../eal/lmice_eal_shm.h"
#include "../eal/lmice_eal_event.h"
#include "../eal/lmice_trace.h"
#include "../eal/lmice_eal_thread.h"
#include "../eal/lmice_eal_spinlock.h"
#include "../rtspace.h"
#include "../resource/resource_manage.h"
}

struct LMMessage
{
    std::string get(int sz);
    int set(const std::string& str);
    void* _addr;
    int _size;
    int _capacity;
};

class lmice_spi:public LMspi
{
public:

    lmice_spi();
    ~lmice_spi();

    //python
    int py_register_callback(uint64_t id, boost::python::object obj);
    int py_get_message(uint64_t id, LMMessage& msg);

    // 辅助函数
    int open_shm(uint64_t hval, lmice_shm_t* shm);
    int init();
    int commit();

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

    //基于ID的回调函数管理
    int register_callback(uint64_t id, lmice_event_callback *callback);
    int unregister_callback(uint64_t id, lmice_event_callback *callback);

    //可信计算,QoS管理
    int set_tc_level(int level);
    int set_qos_level(int level);

    //阻塞运行与资源回收
    int join();



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

    std::map<uint64_t, boost::python::object> m_pylist;
};

#endif /** LMSPI_IMPL_H */

