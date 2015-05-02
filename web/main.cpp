#include "json_request.h"
#include "user.h"

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
#include <signal.h>
#include <sstream>

#include <boost/thread.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::lib::thread;

//std::string get_remote_ip( websocketpp::connection_hdl hdl)
//{
//    websocketpp::lib::error_code ec;
//    server::connection_ptr cp = server::get_con_from_hdl(hdl, ec);
//    return cp->get_socket().remote_endpoint().address().to_string();
//}

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;
//boost::thread_specific_ptr<int64_t> tss_ptr;

//  Define a callback to handle connection close
void on_close(server* s, websocketpp::connection_hdl hdl)
{
    //std::cout<<"erase token "<<*tss_ptr.get()<<"\n";
    int64_t ic = reinterpret_cast<intptr_t>(hdl.lock().get());
    std::cout<<"erase "<<ic<<"\n";
    lmice::token_erase(hdl.lock().get());
    std::cout<<"erase finished\n";

}

// Define a callback to handle incoming connection
void on_open(server* s, websocketpp::connection_hdl hdl)
{
    server::connection_ptr cp = s->get_con_from_hdl(hdl);
    boost::asio::ip::tcp::socket& sock = cp->get_socket();
    std::string local_ip = sock.local_endpoint().address().to_string();
    std::string remote_ip = sock.remote_endpoint().address().to_string();
    lmice::token_insert(hdl.lock().get(), local_ip.c_str(), remote_ip.c_str());

    int64_t ic = reinterpret_cast<intptr_t>(hdl.lock().get());
    //tss_ptr.reset(new int64_t(ic));

    std::cout<<"create "<<ic<<"\n";

}

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;


    try
    {
        lmice::token_update_time(hdl.lock().get());
        //std::cout<<"ip:\t"<<get_remote_ip(hdl)<<std::endl;
        const std::string& payload = msg->get_payload();
        char* response = parse_response(hdl.lock().get(), payload.c_str(), payload.size());
        if(response)
        {
            s->send(hdl, response, strlen(response), websocketpp::frame::opcode::TEXT);
            free(response);
        }

        //s->send(hdl, msg->get_payload(), msg->get_opcode());
    }
    catch(const std::exception& e)
    {
        s->close(hdl,
                 websocketpp::close::status::unsupported_data,
                 e.what());
    }
    catch (const websocketpp::lib::error_code& e)
    {
        std::cout << "Echo failed because: " << e
                  << "(" << e.message() << ")" << std::endl;
    }
}

// 进程退出
volatile sig_atomic_t g_quit = 0;
void handle_quit(server* s )
{
    if(!g_quit)
    {
        g_quit = 1;

        s->stop();
    }
}



int main() {

    // Create asio server pool
    boost::asio::io_service ios(4);

    // 打开配置信息
    lmice::user_load_set();


    try {
        server lmweb_server;


        boost::asio::signal_set signals_(ios, SIGINT, SIGTERM
                                 #if defined(SIGQUIT)
                                         ,SIGQUIT
                                 #endif // defined(SIGQUIT)
                                         );
        // 注册信号量处理函数
        signals_.async_wait(bind(handle_quit, &lmweb_server));


        // 设置日志等级
        lmweb_server.set_access_channels(websocketpp::log::alevel::all);
        lmweb_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
        lmweb_server.set_error_channels(websocketpp::log::elevel::all);

        // 初始化 ASIO
        lmweb_server.init_asio(&ios);

        //        boost::asio::io_service& ios = echo_server.get_io_service();
        //        boost::asio::signal_set signals_(ios, SIGINT, SIGTERM
        //                                     #if defined(SIGQUIT)
        //                                         ,SIGQUIT
        //                                     #endif // defined(SIGQUIT)
        //                                         );
        //        signals_.async_wait(bind(handle_quit, &echo_server));

        // 注册事件回调函数(消息到达, 打开连接,关闭连接)
        lmweb_server.set_message_handler(bind(&on_message,&lmweb_server,::_1,::_2));
        lmweb_server.set_open_handler( bind(&on_open, &lmweb_server, ::_1));
        lmweb_server.set_close_handler( bind(&on_close, &lmweb_server, ::_1));

        // 打开端口7935
        lmweb_server.listen(7935);

        // 开始等待浏览器连接
        lmweb_server.start_accept();

        // 开始运行 ASIO io_service
        lmweb_server.run();


    } catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }

    lmice::user_save_set();
    int sz;
    lmice::token_size(&sz);
    std::cout<<"token size:"<<sz<<"\n";

    return 0;

}
