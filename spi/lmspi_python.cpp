
#include "lmspi_impl.h"
#include "lmspi_cxx.h"
#include <boost/python.hpp>
#include <vector>

#include "eal/lmice_trace.h"

using namespace boost::python;

struct LMEvent
{
    uint64_t getid() const
    {
        return _id;
    }
    void setid(uint64_t id)
    {
        _id = id;
    }

    uint64_t _id;
};

std::string LMMessage::get(int sz)
{
    if(sz == 0)
        sz = _capacity;
    std::string str;
    str.insert(str.begin(), (unsigned char*)_addr, (unsigned char*)_addr+sz );
    return str;
}
int LMMessage::set(const std::string& str)
{
    if(_addr != 0)
    {
        size_t sz = str.length();
        if(sz > _capacity)
            sz = _capacity;
        std::copy(str.begin(), str.begin()+sz, (char*)_addr);
    }
    return 0;
}


struct LMspiWrap : LMspi, wrapper<LMspi>
{
    int init()
    {
        override f = this->get_override("init");
        return f();
    }

    int commit()
    {
        override f = this->get_override("commit");
        return f();
    }

    // 场景管理
    int join_session(uint64_t session_id)
    {
        override f = this->get_override("join_session");
        return f(session_id);
    }

    int leave_session(uint64_t session_id)
    {
        override f = this->get_override("leave_session");
        return f(session_id);
    }

    // 资源注册
    int register_publish(const char* type, const char* inst, int size, uint64_t *event_id)
    {
        override f = this->get_override("register_publish");
        return f(type, inst, size, event_id);
    }

    int register_subscribe(const char* type, const char* inst, uint64_t* event_id)
    {
        override f = this->get_override("register_subscribe");
        return f(type, inst, event_id);
    }

    // 事件管理
    int register_tick_event(int period, int size, int due, uint64_t* event_id)
    {
        override f = this->get_override("register_tick_event");
        return f(period, size, due, event_id);
    }
    int register_timer_event(int period, int size, int due, uint64_t* event_id)
    {
        lmice_debug_print("call LMspiWrap register_timer_event\n");
        override f = this->get_override("register_timer_event");
        return f(period, size, due, event_id);

    }

    int register_custom_event(uint64_t* event_list, size_t count, uint64_t* event_id)
    {
        override f = this->get_override("register_custom_event");
        return f(event_list, count, event_id);
    }

    // 基于ID的回调函数管理
    int register_callback(uint64_t id, lmice_event_callback *callback)
    {
        override f = this->get_override("register_callback");
        return f(id, callback);
    }

    int unregister_callback(uint64_t id, lmice_event_callback *callback)
    {
        override f = this->get_override("unregister_callback");
        return f(id, callback);
    }

    // 可信计算,QoS管理
    int set_tc_level(int level)
    {
        override f = this->get_override("set_tc_level");
        return f(level);
    }

    int set_qos_level(int level)
    {
        override f = this->get_override("set_qos_level");
        return f(level);
    }

    // 阻塞运行与资源回收
    int join()
    {
        override f = this->get_override("leave_session");
        return f();
    }
};

// 资源注册
int pywrap_register_publish(LMspi&spi, const char* type, const char* inst, int size, LMEvent& event_id)
{
    int ret = 0;
    uint64_t eid = 0;
    ret = spi.register_publish(type, inst, size, &eid);
    event_id.setid(eid);
    return ret;
}

int pywrap_register_subscribe(LMspi&spi, const char* type, const char* inst, LMEvent& event_id)
{
    int ret = 0;
    uint64_t eid = 0;
    ret = spi.register_subscribe(type, inst, &eid);
    event_id.setid(eid);
    return ret;
}

// 事件管理
int pywrap_register_tick_event(LMspi&spi, int period, int size, int due, LMEvent& event_id)
{
    int ret = 0;
    uint64_t eid = 0;
    ret = spi.register_tick_event(period, size, due, &eid);
    event_id.setid(eid);
    return ret;
}


int pywrap_register_timer_event(LMspi&spi, int period, int size, int due, LMEvent& event_id)
{
    int ret = 0;
    uint64_t eid = 0;
    ret = spi.register_timer_event(period, size, due, &eid);
    event_id.setid(eid);
    return ret;
}

int pywrap_register_custom_event(LMspi&spi, const std::vector<uint64_t>& event_list, size_t count, LMEvent& event_id)
{
    int ret = 0;
    uint64_t eid = 0;
    ret = spi.register_custom_event(const_cast<uint64_t*>(&event_list[0]), count, &eid);
    event_id.setid(eid);
    return ret;
}

int pywrap_register_callback(LMspi&spi, uint64_t id, object obj)
{
    lmice_debug_print("%llu event register_callback\n", id);
    lmice_spi* lspi = dynamic_cast<lmice_spi*>(&spi);
    lspi->py_register_callback(id, obj);
    return 0;
}

int pywrap_get_mesg(LMspi&spi, LMEvent eid, LMMessage& msg)
{
    int ret = 0;
    lmice_spi* lspi = dynamic_cast<lmice_spi*>(&spi);
    ret = lspi->py_get_message(eid.getid(), msg);
    return ret;
}


BOOST_PYTHON_MODULE(lmspi)
{

    class_<LMMessage>("LMMessage")
            .def("get", &LMMessage::get)
            .def("set", &LMMessage::set)
            ;

    class_<LMEvent>("LMEvent")
            .def_readwrite("id", &LMEvent::_id)
            .def("getid", &LMEvent::getid)
            .def("setid", &LMEvent::setid);


    class_<LMFactory>("LMFactory")
            .def("create_spi", &LMFactory::create_spi, return_value_policy<manage_new_object>())
            .def("delete_spi", &LMFactory::delete_spi);

    class_<LMspiWrap, boost::noncopyable>("LMspi")
            // 辅助函数
            .def("init", pure_virtual(&LMspi::init))
            .def("commit", pure_virtual(&LMspi::commit) )

            // 场景管理
            .def("join_session", pure_virtual(&LMspi::join_session) )
            .def("leave_session", pure_virtual(&LMspi::leave_session) )
            // 资源注册
            .def("register_publish", pure_virtual(&LMspi::register_publish) )
            .def("register_subscribe", pure_virtual(&LMspi::register_subscribe) )

            // 事件管理
            .def("register_tick_event", pure_virtual(&LMspi::register_tick_event) )
            .def("register_timer_event", pure_virtual(&LMspi::register_timer_event) )
            .def("register_custom_event", pure_virtual(&LMspi::register_custom_event) )

            // 基于ID的回调函数管理
            .def("register_callback", pure_virtual(&LMspi::register_callback) )
            .def("unregister_callback", pure_virtual(&LMspi::unregister_callback) )

            // 可信计算,QoS管理
            .def("set_tc_level", pure_virtual(&LMspi::set_tc_level) )
            .def("set_qos_level", pure_virtual(&LMspi::set_qos_level) )

            // 阻塞运行与资源回收
            .def("join", pure_virtual(&LMspi::join) )
            // 重载

            .def("register_publish", pywrap_register_publish )
            .def("register_subscribe", pywrap_register_subscribe )

            .def("register_tick_event", pywrap_register_tick_event )
            .def("register_timer_event", pywrap_register_timer_event)
            .def("register_custom_event", pywrap_register_custom_event )

            .def("register_callback", pywrap_register_callback )
            //.def("unregister_callback", pure_virtual(&LMspi::unregister_callback) )

            .def("get_mesg", pywrap_get_mesg)
            ;

}
