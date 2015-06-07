#ifndef IO_SCHEDULE_H
#define IO_SCHEDULE_H

#include "resource/resource_manage.h"

int create_io_thread(lm_res_param_t* pm);
int stop_io_thread(lm_res_param_t *pm);

#endif /** IO_SCHEDULE_H */
