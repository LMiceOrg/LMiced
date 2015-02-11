/** @copydoc */
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H
/**  @abstract */
/**  @version    @author    @date */

#include <boost/program_options.hpp>
namespace lmice {


class config
{
public:
    explicit config(int argc, char* argv[]);
    int init();
    int process();
    const std::string* get(const char *name) const
    {
        const std::string* pstr = 0;
        boost::program_options::variables_map::const_iterator ite;
        ite = vm.find(name);
        if(ite != vm.end())
            pstr = boost::any_cast<std::string>( &(ite->second.value()) );
        return pstr;
    }
    template<class T>
    const T* get(const char* name) const
    {
        const T* r = 0;

        boost::program_options::variables_map::const_iterator ite;
        ite = vm.find( name);
        if(ite != vm.end())
        {
            r = boost::any_cast<T>( &(ite->second.value()) );
        }
//        if(vm.count(name))
//        {
//            r = boost::any_cast<T>( &(vm[name].value()) );
//        }
        return r;
    }
    size_t unknown_size(const char* name) const;
    std::vector<std::string > unknown_get(const char* name) const;
private:
    boost::program_options::options_description cmdline_options;
    boost::program_options::variables_map vm;
    std::vector<std::string > um;
    int _argc;
    char** _argv;
};

} // namespace lmice

#endif /** SYSTEM_CONFIG_H */
