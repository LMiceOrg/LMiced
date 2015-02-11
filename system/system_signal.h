#ifndef SIGNAL_PROCESS_H
#define SIGNAL_PROCESS_H


#include "system_environment.h"

#include <signal.h>

#define LMICE_SIGNAL_COUNT 4
extern volatile sig_atomic_t lmice_signal_set[LMICE_SIGNAL_COUNT];


enum lmice_signal_e
{
    /* stop lmiced */
    lmice_stop = SIGINT,
    /* reopen lmiced */
    lmice_reopen = SIGHUP,
    /* worker request command */
    lmice_client_command = SIGUSR2,
    /* worker disconnect to lmiced */
    lmice_client_disconnect = SIGUSR1
};

struct lmice_signal_s {
    int signal;
    char name[20];
    char cmd[20];
    int mask;
};
typedef struct lmice_signal_s lmice_signal_t;
extern lmice_signal_t lmice_signal_list[LMICE_SIGNAL_COUNT];

/* Initialize signal */
inline __attribute__((always_inline))
int lmice_signal_init(lmice_environment_t* env);

/* Main thread does process signal loop */
inline __attribute__((always_inline))
int lmice_process_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_stop_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_reopen_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_client_command_signal(lmice_environment_t* env);

inline __attribute__((always_inline))
void lmice_client_disconnect_signal(lmice_environment_t* env);

/* Signal procedure */
void sigproc(int s, siginfo_t *, void *);


#endif /** SIGNAL_PROCESS_H */
