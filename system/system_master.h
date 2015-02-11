#ifndef SYSTEM_MASTER_H
#define SYSTEM_MASTER_H

#include "system_environment_internal.h"

namespace lmice
{


class SystemMaster
{
public:
    SystemMaster(lmice_environment_t* e);

    void CreateDaemon() noexcept;
    int initialize();
    int initSignal();
    int mainLoop();
private:
    lmice_environment_t* env;
};

}

#endif // SYSTEM_MASTER_H
