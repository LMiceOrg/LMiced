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


enum lmice_worker_state_e
{
    WORKER_RUNNING = 1,
    WORKER_MODIFIED = 2,
    WORKER_DEAD = 3
};

struct lmice_shm_resourece_s
{
    evtfd_t efd;
    shmfd_t sfd;
    addr_t  addr;
};
typedef struct lmice_shm_resourece_s lm_shm_res_t;

struct lmice_action_s
{
    uint8_t state[8];
};
typedef struct lmice_action_s lm_action_t;

/**
 * @brief The lmice_action_info_s struct
 * 用户事件描述
 */
struct lmice_action_info_s
{
    uint32_t type;              //join
    uint32_t size;
    uint64_t inst_id;           // 实例编号
    uint64_t act_ids[8];
    lm_action_t state;

};
typedef struct lmice_action_info_s lm_action_info_t;

struct lmice_action_res_s
{
    evtfd_t efd;
    lm_action_info_t *info;
};
typedef struct lmice_action_res_s lm_action_res_t;

/**
 * @brief The lmice_timer_s struct
 * 定时器
 */
struct lmice_timer_s
{
    uint64_t count;             // 已完成触发数量
    int64_t  begin;             // 开始时间
};
typedef struct lmice_timer_s lm_timer_t;

/**
 * @brief The lmice_timer_info_s struct
 * 定时器描述,同时包括定时器运行时状态
 */
struct lmice_timer_info_s
{
    uint32_t type;              //ticker timer
    uint32_t size;               // 触发计数量
    int64_t period;             // 周期长度
    int64_t due;               // 预期开始时间, -1 立即开始, 0 下周期开始
    uint64_t inst_id;           // 实例编号


    lm_timer_t  state;      //定时器状态
};
typedef struct lmice_timer_info_s lm_timer_info_t;

struct lmice_timer_resource_s
{
    evtfd_t efd;
    lm_timer_info_t *info;
};
typedef struct lmice_timer_resource_s lm_timer_res_t;

/**
 * @brief The lmice_message_s struct
 * 消息
 */
struct lmice_message_s
{
    uint32_t lock;  //sync purpose
    uint32_t size;  //blob size( bytes)
    char blob[8];
};
typedef struct lmice_message_s lm_mesg_t;

/**
 * @brief The lmice_message_info_s struct
 * 消息描述
 */
struct lmice_message_info_s
{
    uint32_t        type;       // publish; subscribe by instance or subscribe by type
    uint32_t        size;       // size
    int64_t         period;     // message publish  period tick, zero(0) means no period
    uint64_t inst_id;           // 实例编号 (决定了 响应事件与共享内存编号)
    uint64_t type_id;           // 类型编号
};
typedef struct lmice_message_info_s lm_mesg_info_t;

/**
 * @brief The lmice_message_resource_s struct
 * 消息资源
 */
struct lmice_message_resource_s
{
    lm_shm_res_t res;
    lm_mesg_info_t *info;
};
typedef struct lmice_message_resource_s lm_mesg_res_t;

/**
 * @brief The lmice_worker_s struct
 * 应用软件/模型
 */
struct lmice_worker_s
{
    uint32_t version;
    uint32_t size;
    uint32_t state;
    uint32_t reserved;
    /* next instance identity, zero(0) means no extra instance block */
    uint64_t next_id;

    uint64_t lock;
    uint64_t inst_id;   // 实例编号
    uint64_t type_id;   // 类型编号

    lm_mesg_info_t      mesg[128];
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
    uint32_t state; /* fine modified dead*/

    /* worker process id if equal to zero(0) means rtspace maintain the resource and thread-level instance */
    uint32_t process_id;
    /* worker thread id, if equal to zero(0) means process-level instance */
    uint32_t thread_id;
    /* worker type identity, user defined */
    uint64_t type_id;
    /* worker instance identity */
    uint64_t inst_id;

};
typedef struct lmice_worker_info_s lm_worker_info_t;

struct lmice_worker_resource_s
{
    lm_shm_res_t        res;
    lm_worker_info_t*   info;
    lm_mesg_res_t       mesg[128];
    lm_timer_res_t      timer[128];
    lm_action_res_t     action[128];
};
typedef struct lmice_worker_resource_s lm_worker_res_t;

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


struct lmice_resource_parameter_s
{
    lm_time_param_t tm_param;
    lm_shm_res_t   res_server;
    lm_worker_res_t res_worker[DEFAULT_CLIENT_SIZE];
};
typedef struct lmice_resource_parameter_s lm_res_param_t;



int create_resource_service(lm_res_param_t* pm);
int destroy_resource_service(lm_res_param_t* pm);

#endif /** RESOURCE_MANAGE_H */

