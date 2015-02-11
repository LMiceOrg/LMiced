#include "system_environment_internal.h"
#include "system_environment.h"

#include "lmice_trace.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

#define DEFAULT_LOG_PATH "/var/tmp/lmiced.log"

namespace lmice
{

SystemEnvironment::SystemEnvironment()
{
    _pid = 0;
    _ppid = 0;
    _addr = 0;
    _board = 0;
    _logfd = 0;
    _cfg = 0;
}

SystemEnvironment::~SystemEnvironment()
{
    if(_cfg)
        delete _cfg;

    if(_logfd)
        fclose(_logfd);
}

int SystemEnvironment::processCommandLine(int argc, char **argv)
{
    _cfg = new lmice::config(argc, argv);
    _cfg->init();
    return _cfg->process();
}

int SystemEnvironment::initialize()
{
    int ret = 0;
    const std::string* pfile;
    const char* filename = DEFAULT_LOG_PATH;
    if( (pfile=_cfg->get("log-file")) != NULL)
    {
        filename = pfile->c_str();
    }

    _logfd = fopen(filename, "a+");
    if(_logfd == NULL)
        ret = errno;

    return ret;
}

int SystemEnvironment::role()
{
    if(     _cfg->get("daemon") )
        return 1;
    else if(_cfg->get("force-clean") )
        return 2;
    return 0;
}


}
