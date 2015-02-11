#include "config.h"

#include "lmice_trace.h"

#include <boost/filesystem.hpp>

/** C++ standard */
#include <iostream>
#include <string>
#include <vector>

namespace lmice {

namespace po = boost::program_options;
namespace fs = boost::filesystem;


config::config(int argc, char* argv[])
    :_argc(argc),_argv( reinterpret_cast<char**>(argv) )
{
    //_argv = (char**)argv;

    po::options_description tcp_config("TCP Server configuration");
    tcp_config.add_options()
            ("tcp.enable", po::value<bool>()->default_value(1), "set True(1)/False(0) to enable/disable tcp server\n\tenable = True")
            ("tcp.address", po::value< std::vector<std::string> >(), "set ip address\n\taddress = 127.0.0.1")
            ("tcp.port", po::value<std::vector<std::string> >(), "set ports\n\tports = 8900")
            ("tcp.link", po::value<std::vector<std::string> >(), "links are a set of tcp server\n\tlink= <address>:<port>")
            ;
    po::options_description mc_config("Multicast Server configuration");
    mc_config.add_options()
            ("multicast.enable", po::value<bool>()->default_value(1), "set to enable multicast server")
            ("multicast.address", po::value< std::vector<std::string> >(), "set multicast group address")
            ("multicast.port", po::value<std::vector<int> >(0), "set port")
            ("multicast.listen_address", po::value< std::vector<std::string> >(), "set listen address")
            ("multicast.ttl", po::value<size_t >()->default_value(1), "set time to live(hops)")
            ("multicast.link", po::value<std::vector<std::string> >(), "links are a set of multicast server\n\tlink= <address>:<port>:<listen_address>:<ttl>")
            ;
    po::options_description udp_config("UDP Server configuration");
    udp_config.add_options()
            ("udp.enable", po::value<bool>()->default_value(true), "set to enable multicast server")
            ("udp.address", po::value< std::vector<std::string> >(), "set multicast group address")
            ("udp.port", po::value<std::vector<int> >(), "set port")
            ("udp.link", po::value<std::vector<std::string> >(), "links are a set of udp server\n\tlink= <address>:<port>")
            ;

    po::options_description uds_config("UNIX Domain Socket configuration");
    uds_config.add_options()
            ("uds.enable", po::value<bool>()->default_value(1), "set to enable UNIX Domain Socket server")
            ("uds.address", po::value< std::vector<std::string> >(), "set UNIX Domain Socket addresses")
            ;

    po::options_description lfilter_config("Local Filter configuration");
    lfilter_config.add_options()
            ("localfilter.ids", po::value<std::vector<std::string> >(), "set the ids of local filters")
            ;
    cmdline_options.add(tcp_config)
            .add(mc_config)
            .add(udp_config)
            .add(uds_config)
            .add(lfilter_config)
            .add_options()
            ("force-clean,c", "force clean")
            ("daemon,d", "run daemon in backend")
            ("log-file,l", po::value<std::string>()->default_value("/var/tmp/lmiced.log"), "log file")
            ("threads", po::value<size_t>()->default_value(0), "set the number of server threads\n"
             "0 present the number of CPUs")
            ("help,h", "Show this help message")
            ("test", "Test configuration")
            ("version,v", "Show version message")
            ("clean","Force clean existing shmm")
            ("config-file,f", po::value< std::vector<std::string> >(), "Set config file")
            ("list-stock-id", "List all stock ids")
            ("si","signals")
            ("signal,S",po::value< std::string >(), "Send signal to Master process"
             "\nstop:\tshutdown the server"
             "\nappend <sid>:\tadd and start querying tick"
             "\nremove <sid>:\tremove and stock querying tick"
             "\nreload:\treload config file"
             "\neg:\t-s append sh600001 sh600002: start querying sh600001,sh600002 tick data")
            ("stock-id,I",po::value< std::vector< std::string> >()->composing(), "stock id\neg:-I sh600123")
            ;
}

int config::init()
{
    // Clear existing config setting
    vm.clear();
    um.clear();

    po::positional_options_description pp;
    pp.add("stock-id",-1);
    po::parsed_options parsed = po::basic_command_line_parser<char>(_argc, _argv)
              .options(cmdline_options)
              .allow_unregistered().positional(pp).run();
    um = po::collect_unrecognized<char>(parsed.options, po::exclude_positional);
    po::store(parsed, vm);
    //po::notify(vm);
    if(vm.count("config-file"))
    {
        const boost::any &v = vm["config-file"].value();
        const std::vector<std::string>* pvs = boost::any_cast<std::vector<std::string> >( &v );
        if(pvs)
        {
            for(std::vector<std::string>::const_iterator ite = pvs->begin();
                ite != pvs->end(); ++ite)
            {
                fs::path p(*ite);
                if(fs::exists(p)
                        && fs::is_regular_file(p))
                {
                    lmice_debug_print("config file is %s", p.string().c_str());
                    po::parsed_options pos = po::parse_config_file<char>(p.string().c_str(), cmdline_options, true);
                    po::store(pos, vm);
                    //po::store(po::parse_config_file<char>(p.string().c_str(), cmdline_options, true), vm);
                    po::notify(vm);


                }
            }
        }
    }

    std::string default_conf = _argv[0];
    fs::path p0(default_conf);
    //p = fs::absolute(default_conf);
    p0.remove_filename() /= "lmiced.conf";
    fs::path p = p0;//fs::absolute(p0);
    if(fs::exists(p)
            && fs::is_regular_file(p))
    {
        po::parsed_options pos = po::parse_config_file<char>(p.string().c_str(), cmdline_options, true);
        po::store(pos, vm);


    }

    po::notify(vm);
    return 0;

}

int config::process()
{
    if (vm.count("help"))
    {
        lmice_info_print("LMiced Help\n");
        std::cout<<cmdline_options<<"\n";
        return 1;
    }
    else if(vm.count("version"))
    {
        lmice_info_print("LMiced Version Information\n"
                         "LMiced: Daemon of Lmice Message Interactive Computing Environment\n"
                         "Version: 1.0.123\n");
        return 1;
    }
    else if(vm.count("test"))
    {
        lmice_info_print("Config Test\n");
        std::map<std::string, po::variable_value>::const_iterator ite;
        std::cout<<" localfilter1.file size is:"<<unknown_size("localfilter1.file")<<"\n";
        for(size_t i=0; i< um.size(); ++i)
        {
            std::cout<<"unrecognized option:"<<um[i]<<"\n";
        }
        std::cout<<"total param size:"<<vm.size()<<"\n";
        for(ite = vm.begin(); ite != vm.end(); ++ite)
        {
            std::cout<<ite->first<<"\t";
            const boost::any& v = ite->second.value();
            const bool *pb;
            const size_t* ps;
            const std::vector<int> *pvi;
            const std::vector<std::string> *pvs;
            if( (pb=boost::any_cast<bool>(&v)) != 0)
                std::cout<<*pb<<"\n";
            else if( (ps=boost::any_cast<size_t>(&v)) != 0)
                std::cout<<*ps<<"\n";
            else if( (pvi=boost::any_cast< std::vector<int> >(&v)) != 0)
            {
                std::cout<<"total:"<<pvi->size()<<"\n";
                int i = 0;
                for(std::vector<int>::const_iterator cv_ite =
                    pvi->begin(); cv_ite != pvi->end(); ++cv_ite, ++i)
                    std::cout<<"\t"<<i<<":\t"<<*cv_ite<<"\n";
            }
            else if( (pvs=boost::any_cast< std::vector<std::string> >(&v)) != 0)
            {
                std::cout<<"total:"<<pvs->size()<<"\n";
                int i = 0;
                for(std::vector<std::string>::const_iterator cv_ite =
                    pvs->begin(); cv_ite != pvs->end(); ++cv_ite, ++i)
                    std::cout<<"\t"<<i<<":\t"<<*cv_ite<<"\n";
            }
            else
                std::cout<<"\n";
        }
        return 1;
    }

    return 0;
}

size_t config::unknown_size(const char *name) const
{
    size_t cnt = 0;
    std::vector<std::string>::const_iterator ite;
    for(ite = um.begin(); ite != um.end(); ++ite)
    {
        ite = std::find(ite, um.end(), name);
        if(ite != um.end())
        {
            ++cnt;
        }
        else
        {
            break;
        }

    }
    return cnt;
}

std::vector<std::string > config::unknown_get(const char *name) const
{
    std::vector<std::string > vec;
    std::vector<std::string>::const_iterator ite;
    for(ite = um.begin(); ite != um.end(); ++ite)
    {
        ite = std::find(ite, um.end(), name);
        if(ite != um.end() && (ite+1) != um.end())
        {
            vec.push_back(*(ite+1));
        }
        else
        {
            break;
        }
    }
    return vec;
}

} // namespace lmice
