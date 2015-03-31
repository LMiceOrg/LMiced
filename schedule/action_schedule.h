#ifndef ACTION_SCHEDULE_H
#define ACTION_SCHEDULE_H



/** 任务调度服务
 * 定时器事件:
 *  存储引用它的用户事件链表,并在状态改变时触发其相关用户事件检查
 * 资源事件:
 *  存储引用它的用户事件链表,并在状态改变时触发其相关用户事件检查
 * 用户事件:
 *  不做主动处理
 *
*/

#include "resource/resource_manage.h"

int create_schedule_service(lm_res_param_t* pm);
int destroy_schedule_service(lm_res_param_t* pm);

#endif /* ACTION_SCHEDULE_H */

