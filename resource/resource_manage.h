#ifndef RESOURCE_MANAGE_H
#define RESOURCE_MANAGE_H

/** 资源包括:
 * 1.工作实例(进程级,线程级)列表;
 * 2.系统墙上时间,仿真时间.
 *
 * 工作实例资源包括:
 * 1.发布订阅的(共享内存)信息;
 * 2.(墙上时间/仿真时间)定时器(周期性,一次性);
 * 3.自定义事件.
 *
**/
#include "eal/lmice_eal_shm.h"
#include "eal/lmice_eal_event.h"
#include "eal/lmice_eal_thread.h"

#include "timer/timer_system_time.h"

#include <stdint.h>

#define CLIENT_SHMNAME  "CC597303-0F85-40B2-8BDC-4724BD" /** C87B4E */
#define BOARD_SHMNAME   "82E0EE49-382C-40E7-AEA2-495999" /** 392D29 */
#define DEFAULT_SHM_SIZE 4096 /** 4KB */
#define DEFAULT_CLIENT_SIZE 200

/**
 * @brief The lmice_action_info_s struct
 * 用户事件
 */
struct lmice_action_info_s
{
    uint32_t type;              //join
    uint32_t size;
    int32_t state;
    int32_t reserved;
    uint64_t inst_id;           // 实例编号
    uint64_t act_ids[16];

};
typedef struct lmice_action_info_s lm_action_info_t;


/**
 * @brief The lmice_timer_info_s struct
 * 定时器事件
 */
struct lmice_timer_info_s
{
    uint32_t type;              //ticker timer
    int32_t state;              // 状态
    int32_t size;               // 触发计数量
    int32_t period;             // 周期长度
    uint64_t inst_id;            // 实例编号

    // 状态量
    uint64_t count;             // 已完成触发数量
    int64_t begin;              // 开始时间
};
typedef struct lmice_timer_info_s lm_timer_info_t;

/**
 * @brief The lmice_message_info_s struct
 * 发布消息
 */
struct lmice_publish_message_info_s
{
    uint32_t        type;   // publish
    uint32_t        size;   //
    int64_t         tick;   // message tick time
    uint64_t inst_id;       // 实例编号 (决定了 响应事件与共享内存编号)
    uint64_t type_id;       // 类型编号
};
typedef struct lmice_publish_message_info_s lm_pubmsg_info_t;

struct lmice_subscribe_message_info_s
{
    uint32_t type;  // subscribe by instance; subscribe by type
    uint32_t reserved;
    int64_t         tick;   // message tick time
    uint64_t inst_id;       // 实例编号 (决定了 响应事件与共享内存编号)
    uint64_t type_id;       // 类型编号
};
typedef struct lmice_subscribe_message_info_s lm_submsg_info_t;

/**
 * @brief The lmice_worker_s struct
 * 应用软件/模型
 */
struct lmice_worker_s
{
    uint32_t version;
    uint32_t size;
    /* next instance identity, zero(0) means no extra instance block */
    uint64_t next_id;

    uint64_t lock;
    uint64_t inst_id;   // 实例编号
    uint64_t type_id;   // 类型编号

    lm_pubmsg_info_t      pub[128];
    lm_timer_info_t     timer[128];
    lm_action_info_t    action[128];
};
typedef struct lmice_worker_s lm_worker_t;

/**
 * @brief The lmice_worker_info_s struct
 * (pid, tid) :
 *      1. (0,          0) --> empty instance and reusable
 *      2. (0,   not-zero) --> rtspace maintain, thread-level instance
 *      3. (not-zero,   0) --> process-level instance
 *      4. (not-zero,   not-zero) --> thread-level instance
 */
struct lmice_worker_info_s
{
    /* The instance version*/
    uint32_t version;
    uint32_t reserved;

    /* worker process id if equal to zero(0) means rtspace maintain the resource and thread-level instance */
    uint32_t process_id;
    /* worker thread id, if equal to zero(0) means process-level instance */
    uint32_t thread_id;
    /* worker type identity, user defined */
    uint64_t type_id;
    /* worker instance identity */
    uint64_t instance_id;

};
typedef struct lmice_worker_info_s lm_worker_info_t;

struct lmice_server_s
{
    /* always be 1, the info struct version */
    uint32_t version;
    /* server shm block size*/
    uint32_t size;
    /* next server info identity, zero(0) means no extra server info block */
    uint64_t next_id;

    /* lock */
    uint64_t lock;
    /* event identity */
    uint64_t event_id;
    /* system/tick time resource */
    lm_time_t tm;
    /* worker(client) instances info list */
    lm_worker_info_t worker;
};
typedef struct lmice_server_s lm_server_t;

struct lmice_instance_resourece_s
{
    evtfd_t efd;
    shmfd_t sfd;
    addr_t  addr;
};
typedef struct lmice_instance_resourece_s lm_inst_res_t;


struct lmice_worker_resource_s
{
    pid_t process_id;
    pid_t thread_id;
    lm_inst_res_t  res;
};
typedef struct lmice_worker_resource_s lm_worker_res_t;


struct lmice_resource_parameter_s
{
    lm_time_param_t tm_param;
    lm_inst_res_t   res_server;
    lm_worker_res_t res_worker[DEFAULT_CLIENT_SIZE];
};
typedef struct lmice_resource_parameter_s lm_res_param_t;



int create_resource_service(lm_res_param_t* res);
int destroy_resource_service(lm_res_param_t* res);

#endif /** RESOURCE_MANAGE_H */

