#include "system_master.h"
#include "system_signal.h"

#include "lmice_eal_shm.h"
#include "lmice_trace.h"

#include <lmice_eal_daemon.h>

#include <signal.h>
#include <errno.h>


namespace lmice
{

SystemMaster::SystemMaster(lmice_environment_t *e)
    :env(e)
{

}

void SystemMaster::CreateDaemon() noexcept
{
    eal_create_daemon();
}

int SystemMaster::initialize()
{
    int ret;
    lmice_shm_t control_shm = {0, DEFAULT_SHM_SIZE, 0, CONTROL_SHMNAME};
    lmice_shm_t public_shm  = {0, DEFAULT_SHM_SIZE, 0, PUBLIC_SHMNAME};

    ret = eal_shm_create(&control_shm);
    if(ret != 0)
        return ret;
    ret = eal_shm_create(&public_shm);
    if(ret != 0)
    {
        eal_shm_close(&control_shm);
        return ret;
    }

    env->addr() = control_shm.addr;
    env->board() = public_shm.addr;
    env->pid() = getpid();
    env->ppid() = 0;

    memcpy(reinterpret_cast<void*>(env->board()), &env->pid(), sizeof(pid_t));

    return 0;
}

int SystemMaster::initSignal()
{
    struct sigaction sa;

    for(size_t i=0; i<sizeof(lmice_signal_list)/sizeof(lmice_signal_t); i++)
    {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = sigproc;
        //sigemptyset(&sa.sa_mask);

        if (sigaction(lmice_signal_list[i].signal, &sa, NULL) == -1)
        {
            lmice_error_log(env,"sigaction (%s) got error(%d)", lmice_signal_list[i].name,errno);

            return -1;
        }
    }
    return 0;
}

int SystemMaster::mainLoop()
{
    sigset_t           set;

    sigemptyset(&set);
    sigaddset(&set, lmice_stop);
    sigaddset(&set, lmice_client_command);
    sigaddset(&set, lmice_client_disconnect);
    sigaddset(&set, lmice_reopen);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        //lmice_error_log(env, "sigprocmask() failed");
        lmice_error_print("sigprocmask() failed");
        return -1;
    }

    sigemptyset(&set);

    //lmice_info_log(env, "master thread loop start");
    for(;;)
    {
        sigsuspend(&set);
        if(lmice_signal_set[0] == 1)
        {
            lmice_signal_set[0] = 0;
            //lmice_info_log(env, "got stop signal");
            //lmice_env_exit(env);
            break;
        }
        if(lmice_signal_set[1] == 1)
        {
            lmice_signal_set[1] = 0;
            //lmice_append_signal(env);
            //g_append = 0;
            //lmice_info_log(env, "got append stock id signal");
        }
        if(lmice_signal_set[2] == 1)
        {
            lmice_signal_set[2] = 0;
            //lmice_remove_signal(env);
            //g_remove = 0;
            //lmice_info_log(env, "got remove stock id signal");

        }
        if(lmice_signal_set[3] == 1)
        {
            lmice_signal_set[3] = 0;
            //g_reopen = 0;
            //lmice_info_log(env, "got reopen signal");

        }
    }

    return 0;
}

}
