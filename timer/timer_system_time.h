#ifndef TIMER_SYSTEM_TIME_H
#define TIMER_SYSTEM_TIME_H

#include "resource/resource_manage.h"

#include <eal/lmice_eal_common.h>
#include <eal/lmice_eal_time.h>

int create_time_thread(lm_res_param_t* pm);
int stop_time_thread(lm_res_param_t *pm);

#endif /** TIMER_SYSTEM_TIME_H */

