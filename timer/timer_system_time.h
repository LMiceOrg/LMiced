#ifndef TIMER_SYSTEM_TIME_H
#define TIMER_SYSTEM_TIME_H

#include <stdint.h>

/**
 * @brief stread
 * 获取系统时间
 */
int64_t system_time_read();

/**
 * @brief tcread 获取仿真时间
 * @return 返回仿真时间
 */
int64_t tick_time_read();

int tick_rate_write(int64_t rate);
int64_t tick_rate_read();

#endif // TIMER_SYSTEM_TIME_H

