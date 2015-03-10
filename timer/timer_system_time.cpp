#include "timer_system_time.h"
#include <eal/lmice_eal_common.h>

/**
 * @brief tick_time*tick_rate = system_time
 */
int64_t system_time;
int64_t tick_time;
int64_t tick_rate;

int64_t system_time_read()
{
    return system_time;
}


int64_t tick_time_read()
{
    return tick_time;
}


int tick_rate_write(int64_t rate)
{
    tick_rate = rate;
}


int64_t tick_rate_read()
{
    return tick_rate;
}
