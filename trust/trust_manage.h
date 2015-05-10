#ifndef TRUST_MANAGE_H
#define TRUST_MANAGE_H

#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_time.h"
#include "resource/resource_manage.h"

#include <stdint.h>

#if defined(_WIN32)
struct lmice_trust_s
{
    evtfd_t     efd;
    lm_server_t *server;
    MMRESULT    wTimerID;
    UINT        wTimerRes;
    UINT        wTimerDelay;
};
#elif defined(__LINUX__)

#include <unistd.h>
#include <signal.h>
#include <time.h>

struct time_param_s
{
    lm_server_info_t *server;
    time_t     timerid;
    sigset_t    mask;
    int         sig;
    int         clockid;
};

#elif defined(__MACH__)

struct time_param_s {

};

#endif
typedef struct lmice_trust_s lm_trust_t;

int create_trust_thread(lm_trust_t* pt);
int stop_trust_thread(lm_trust_t* pt);

#endif /** TRUST_MANAGE_H */

