#ifndef SYSTEM_ENVIRONMENT_INTERNAL_H
#define SYSTEM_ENVIRONMENT_INTERNAL_H

#include "config.h"

#include <stdio.h>

#define CONTROL_SHMNAME "CC597303-0F85-40B2-8BDC-4724BD" /** C87B4E */
#define PUBLIC_SHMNAME "82E0EE49-382C-40E7-AEA2-495999" /** 392D29 */
#define DEFAULT_SHM_SIZE 4096 /** 4KB */

namespace lmice
{

class SystemEnvironment
{
public:
    SystemEnvironment();
    ~SystemEnvironment();

    int processCommandLine(int argc, char** argv);
    int initialize();
    int role();
    inline pid_t& pid() noexcept
    {
        return _pid;
    }
    inline pid_t& ppid() noexcept
    {
        return _ppid;
    }
    inline intptr_t& addr() noexcept
    {
        return _addr;
    }
    inline intptr_t& board() noexcept
    {
        return _board;
    }
    inline FILE* logfd() noexcept
    {
        return _logfd;
    }
    inline lmice::config* cfg() noexcept
    {
        return _cfg;
    }
private:
    pid_t _pid;      /* lmiced process id */
    pid_t _ppid;     /* parent process id */
    intptr_t _addr;  /* lmiced shared memory */
    intptr_t _board; /* command shared memory */
    FILE* _logfd;
    lmice::config* _cfg;
};

}

typedef lmice::SystemEnvironment lmice_environment_t;

#endif /** SYSTEM_ENVIRONMENT_INTERNAL_H */
