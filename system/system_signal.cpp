#include "system_environment_internal.h"
#include "system_signal.h"
#include "system_environment.h"
#include "system_worker.h"

/* EAL headers */
#include "lmice_trace.h"
#include "lmice_eal_align.h"

#include <errno.h>
#include <string.h>
#include <signal.h>

#define LMICE_SIGNAL_COUNT 4
volatile sig_atomic_t lmice_signal_set[LMICE_SIGNAL_COUNT];



//enum lmice_signal_e
//{
//    /* stop lmiced */
//    lmice_stop = SIGINT,
//    /* reopen lmiced */
//    lmice_reopen = SIGHUP,
//    /* worker request command */
//    lmice_client_command = SIGUSR2,
//    /* worker disconnect to lmiced */
//    lmice_client_disconnect = SIGUSR1
//};


//struct lmice_signal_s {
//    int signal;
//    char name[20];
//    char cmd[20];
//    int mask;
//};

typedef struct lmice_signal_s lmice_signal_t;

lmice_signal_t lmice_signal_list[]=
{
    {lmice_stop,                "Lmiced Stop",          "stop",     0},
    {lmice_reopen,              "Lmiced Reopen",        "reopen",   1},
    {lmice_client_command,      "Client Command",       "command",  2},
    {lmice_client_disconnect,   "Client Disconnect",    "disc",     3}

};

/* Initialize signal */
inline __attribute__((always_inline))
int lmice_signal_init(lmice_environment_t* env)
{
    struct sigaction sa;
    (void)env;

    memset(&sa, 0, sizeof(struct sigaction));
    for(size_t i=0; i<sizeof(lmice_signal_list)/sizeof(lmice_signal_t); i++)
    {

        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = sigproc;
        if (sigaction(lmice_signal_list[i].signal, &sa, NULL) == -1)
        {
            lmice_error_log(env,"sigaction (%s) got error(%d)", lmice_signal_list[i].name,errno);

            return -1;
        }
    }
    return 0;

}

/* Main thread does process signal loop */
inline __attribute__((always_inline))
int lmice_process_signal(lmice_environment_t* env)
{
    int ret;
    const char* sig = env->cfg()->get<char>("signal");
    if(sig)
    {
        LMICE_TRACE_COLOR_PRINT(lmice_trace_info, "got signal:[%s]", sig);

        ret = lmice_worker_init(env);
        if(ret != 0)
        {
            LMICE_TRACE_COLOR_PRINT(lmice_trace_warning, "worker init failed[%d]", ret);
            return ret;
        }

        if(strcmp(sig, "stop") == 0)
        {

            lmice_info_log(env, "stopping lmiced");
            kill(env->ppid(), lmice_stop);
        }
        else if( strcmp(sig, "reopen") == 0)
        {
            lmice_info_log(env, "reopen lmiced");
            kill(env->ppid(), lmice_reopen);
        }
        else if( strcmp(sig, "command") == 0)
        {
            lmice_info_log(env, "client command");
            kill(env->ppid(), lmice_client_command);
        }
        else if( strcmp(sig, "disconnect") == 0)
        {
            lmice_info_log(env, "client disconnect");
            kill(env->ppid(), lmice_client_disconnect);
        }

        return 1;
    }
    return 0;
}

inline __attribute__((always_inline))
void lmice_stop_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_reopen_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_client_command_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_client_disconnect_signal(lmice_environment_t* env);


/* Signal procedure */
void sigproc(int s, siginfo_t* sig, void * v)
{
    (void)sig;
    (void)v;

    COMPILE_TIME_ASSERT(sizeof(lmice_signal_list)/sizeof(lmice_signal_t)== LMICE_SIGNAL_COUNT);

    for(size_t i=0; i<sizeof(lmice_signal_list)/sizeof(lmice_signal_t); i++)
    {
        if(lmice_signal_list[i].signal == s)
        {
            lmice_signal_set[i] = 1;
            break;
        }
    }
}
