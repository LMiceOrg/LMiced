#ifndef TIMER_SYSTEM_TIME_H
#define TIMER_SYSTEM_TIME_H

#include <stdint.h>

/**
 * @brief stread
 * 获取系统时间
 */
int64_t stread();

/**
 * @brief tcread 获取仿真时间
 * @return 返回仿真时间
 */
int64_t tcread();

int reset_tick_rate(int64_t rate);
int64_t tick_rate();

#endif // TIMER_SYSTEM_TIME_H

