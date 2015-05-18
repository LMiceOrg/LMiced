#ifndef LMICE_CORE_H
#define LMICE_CORE_H
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

struct lmice_environment_s
{
    void* addr;
    void* board;
    int32_t pid;
    int32_t ppid;
    FILE* logfd;
    uint32_t lcore;
    uint32_t memory;
    uint32_t net_bandwidth;
};

typedef struct lmice_environment_s lmice_environment_t;


int eal_env_init(lmice_environment_t* env);

/**
 * @brief lmice_process_cmdline
 * @param argc
 * @param argv
 * @return 0 if success;else error occurred.
 */
int lmice_process_cmdline(int argc, char * const *argv);
int lmice_process_signal(lmice_environment_t* env);
int lmice_exec_command(lmice_environment_t* env);
int lmice_master_init(lmice_environment_t* env);
int lmice_signal_init(lmice_environment_t* env);
void lmice_master_loop(lmice_environment_t* env);
#endif /** LMICE_CORE_H */
