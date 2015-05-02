#include "lmspi_cxx.h"
#include <boost/python.hpp>
using namespace boost::python;

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

    //场景管理
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

    //资源注册
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

    //事件管理
    int register_tick_event(int period, int size, int due, uint64_t* event_id)
    {
        override f = this->get_override("register_tick_event");
        return f(period, size, due, event_id);
    }

    int register_timer_event(int period, int size, int due, uint64_t* event_id)
    {
        override f = this->get_override("register_timer_event");
        return f(period, size, due, event_id);
    }

    int register_custom_event(uint64_t* event_list, size_t count, uint64_t* event_id)
    {
        override f = this->get_override("register_custom_event");
        return f(event_list, count, event_id);
    }

    //基于ID的回调函数管理
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

    //可信计算,QoS管理
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

    //阻塞运行与资源回收
    int join()
    {
        override f = this->get_override("leave_session");
        return f();
    }
};

BOOST_PYTHON_MODULE(lmspi)
{
    class_<LMFactory>("LMFactory")
            .def("create_spi", &LMFactory::create_spi, return_value_policy<manage_new_object>())
            .def("delete_spi", &LMFactory::delete_spi);

    class_<LMspiWrap, boost::noncopyable>("LMspi")
            // 辅助函数
            .def("init", pure_virtual(&LMspi::init))
            .def("commit", pure_virtual(&LMspi::commit) )

            //场景管理
            .def("join_session", pure_virtual(&LMspi::join_session) )
            .def("leave_session", pure_virtual(&LMspi::leave_session) )
            //资源注册
            .def("register_publish", pure_virtual(&LMspi::register_publish) )
            .def("register_subscribe", pure_virtual(&LMspi::register_subscribe) )

            //事件管理
            .def("register_tick_event", pure_virtual(&LMspi::register_tick_event) )
            .def("register_timer_event", pure_virtual(&LMspi::register_timer_event) )
            .def("register_custom_event", pure_virtual(&LMspi::register_custom_event) )

            //基于ID的回调函数管理
            .def("register_callback", pure_virtual(&LMspi::register_callback) )
            .def("unregister_callback", pure_virtual(&LMspi::unregister_callback) )

            //可信计算,QoS管理
            .def("set_tc_level", pure_virtual(&LMspi::set_tc_level) )
            .def("set_qos_level", pure_virtual(&LMspi::set_qos_level) )

            //阻塞运行与资源回收
            .def("join", pure_virtual(&LMspi::join) )
            ;

}
