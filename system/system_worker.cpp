#include "system_environment_internal.h"
#include "system_worker.h"

#include "lmice_eal_shm.h"
#include "lmice_trace.h"

#include <string.h>
#include <signal.h>



namespace lmice
{

SystemWorker::SystemWorker(lmice_environment_t *e)
    :env(e)
{

}

int SystemWorker::processSignal()
{

}

}

//inline __attribute__((always_inline))
int lmice_worker_init(lmice_environment_t *env)
{
    int ret;
    lmice_shm_t control_shm = {0, DEFAULT_SHM_SIZE, 0, CONTROL_SHMNAME};
    lmice_shm_t public_shm  = {0, DEFAULT_SHM_SIZE, 0, PUBLIC_SHMNAME};

    ret = eal_shm_open_readwrite(&control_shm);
    if(ret != 0)
    {
        lmice_debug_print("open control_shm failed\n");
        return ret;
    }
    ret = eal_shm_open_readonly(&public_shm);
    if(ret != 0)
    {
        lmice_debug_print("open public_shm failed\n");
        eal_shm_close(&control_shm);
        return ret;
    }

//    env->addr = control_shm.addr;
//    env->board = public_shm.addr;
//    env->pid = getpid();
//    env->ppid = *(pid_t*)env->board;
    return 0;
}

//inline __attribute__((always_inline))
//int lmice_worker_release(lmice_environment_t *env)
//{

//}

//inline __attribute__((always_inline))
int lmice_master_init(lmice_environment_t *env)
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

//    env->addr = control_shm.addr;
//    env->board = public_shm.addr;
//    env->pid = getpid();
//    env->ppid = 0;
//    memcpy(reinterpret_cast<void*>(env->board), &env->pid, sizeof(pid_t));

    return 0;
}


int lmice_force_clean(lmice_environment_t *)
{
    lmice_shm_t control_shm = {0, DEFAULT_SHM_SIZE, 0, CONTROL_SHMNAME};
    lmice_shm_t public_shm  = {0, DEFAULT_SHM_SIZE, 0, PUBLIC_SHMNAME};
    eal_shm_destroy( &control_shm);
    eal_shm_destroy(&public_shm);
    return 0;
}



void lmice_master_loop(lmice_environment_t* env)
{
    sigset_t           set;

    sigemptyset(&set);
    sigaddset(&set, lmice_stop);
    sigaddset(&set, lmice_appendcode);
    sigaddset(&set, lmice_removecode);
    sigaddset(&set, lmice_reopen);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        LMICE_TRACE(env, lmice_trace_error, "sigprocmask() failed");
        LMICE_TRACE_COLOR_PRINT(lmice_trace_error, "sigprocmask() failed");
        return;
    }

    sigemptyset(&set);

    LMICE_TRACE(env, lmice_trace_info, "master thread loop start");
    for(;;)
    {
        sigsuspend(&set);
        if(g_quit == 1)
        {
            LMICE_TRACE(env, lmice_trace_info, "got stop signal");
            lmice_env_exit(env);
            break;
        }
        if(g_append == 1)
        {
            lmice_append_signal(env);
            g_append = 0;
            LMICE_TRACE(env, lmice_trace_info, "got append stock id signal");
        }
        if(g_remove == 1)
        {
            lmice_remove_signal(env);
            g_remove = 0;
            LMICE_TRACE(env, lmice_trace_info, "got remove stock id signal");

        }
        if(g_reopen == 1)
        {
            g_reopen = 0;
            LMICE_TRACE(env, lmice_trace_info, "got reopen signal");

        }
    }
}
