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

#include <sglib.h>

#include <stdint.h>

#define PUBLISH_RESOURCE_TYPE  1
#define SUBSCRIBE_RESOURCE_TYPE  2
#define TICKER_TYPE 1
#define TIMER_TYPE 2

#define CLIENT_SHMNAME  "CC597303-0F85-40B2-8BDC-4724BD" /** C87B4E */
#define BOARD_SHMNAME   "82E0EE49-382C-40E7-AEA2-495999" /** 392D29 */
#define DEFAULT_SHM_SIZE 4096 /** 4KB */
#define DEFAULT_CLIENT_SIZE 200

enum schedule_state
{
    LM_PENDING_STATE = 0,
    LM_TRIGGERED_STATE = 0xFF

};

enum lmice_worker_state_e
{
    WORKER_RUNNING = 1,
    WORKER_MODIFIED = 2,
    WORKER_DEAD = 3
};

struct lmice_time_s
{
    int64_t system_time;
    /* 10000 --> 1:1,  100 --> 0.01, 1e6 --> 100:1 */
    int64_t tick_rate;
    int64_t tick_time;
    int64_t tick_zero_time;
};
typedef struct lmice_time_s lm_time_t;

struct lmice_state_s
{
    uint8_t value[8];
};
typedef struct lmice_state_s lm_state_t;

struct lmice_shm_resourece_s
{
    evtfd_t efd;
    shmfd_t sfd;
    addr_t  addr;
};
typedef struct lmice_shm_resourece_s lm_shm_res_t;


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
    lm_state_t state;

};
typedef struct lmice_action_info_s lm_action_info_t;


struct lmice_referenced_acion_s
{
    uint32_t pos;
    lm_action_info_t                *info;
    struct lmice_referenced_acion_s *next;
};
typedef struct lmice_referenced_acion_s lm_ref_act_t;

struct lmice_action_res_s
{
    int active;
    lm_action_info_t *info;
    uint64_t work_id;
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
    lm_state_t state;
};
typedef struct lmice_timer_s lm_timer_t;

/**
 * @brief The lmice_timer_info_s struct
 * 定时器描述,同时包括定时器运行时状态
 */
struct lmice_timer_info_s
{
    uint32_t type;              //ticker timer
    uint32_t size;               // 触发计数量 1 --> one-shot  0 --> infinite
    int64_t period;             // 周期长度
    int64_t due;               // 预期开始时间, -1 立即开始, 0 下周期开始
    uint64_t inst_id;           // 实例编号


    lm_timer_t  timer;      //定时器状态
};
typedef struct lmice_timer_info_s lm_timer_info_t;

struct lmice_worker_resource_s;
struct lmice_timer_resource_s
{
    int64_t active;
    lm_timer_info_t *info;
    lm_ref_act_t    *alist;
    struct lmice_worker_resource_s* worker;
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
    lm_state_t state;           //状态
};
typedef struct lmice_message_info_s lm_mesg_info_t;

/**
 * @brief The lmice_message_resource_s struct
 * 消息资源
 */
struct lmice_message_resource_s
{
    //int64_t active;
    addr_t addr;
    shmfd_t sfd;
//    lm_shm_res_t    res;
    lm_mesg_info_t  *info;
    lm_ref_act_t    *alist;
    uint64_t        worker_id;
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


struct lmice_timer_list_s
{
    lm_worker_res_t     *res_worker;    //worker pos
    lm_timer_res_t      *res_timer;
};
typedef struct lmice_timer_list_s lm_tmlist_t;

struct lmice_message_list_s
{
    lm_worker_res_t *res_worker;
    lm_mesg_res_t   *res_mesg;
};
typedef struct lmice_message_list_s lm_msglist_t;


#ifdef _WIN32
struct lmice_time_parameter_s
{
    lm_time_t*  pt;
    MMRESULT    wTimerID;
    UINT        wTimerRes;
    UINT        wTimerDelay;
    uint64_t    count;

};
#elif defined(__LINUX__) || defined(__APPLE__)

#include <unistd.h>
#include <signal.h>
#include <time.h>

struct lmice_time_parameter_s
{
    timer_t     timerid;
    sigset_t    mask;
    int         sig;
    int         clockid;
    lm_time_t*  pt;
    uint64_t    count;
};

#else
#error(No time_param implementation!)
#endif
typedef struct lmice_time_parameter_s lm_time_param_t;

#define TIMER_LIST_SIZE 32
#define TIMER_LIST_NEXT_POS 31
#define PERIOD_1 80000LL
#define PERIOD_2 320000LL
#define PERIOD_3 1280000LL

struct lmice_resource_parameter_s
{

    /* for resource service */
    lm_shm_res_t    res_server;
    lm_worker_res_t res_worker[DEFAULT_CLIENT_SIZE];

    /* for scheduler time maintain */
    lm_time_param_t tm_param;

    /* for timer-scheduler pointer-array */
    lm_timer_res_t* timer_duelist[TIMER_LIST_SIZE];
    lm_timer_res_t* ticker_duelist[TIMER_LIST_SIZE];
    /* <8 milli-second */
    lm_timer_res_t *timer_worklist1[TIMER_LIST_SIZE];
    lm_timer_res_t *ticker_worklist1[TIMER_LIST_SIZE];

    /* <32 milli-second */
    lm_timer_res_t *timer_worklist2[TIMER_LIST_SIZE];
    lm_timer_res_t *ticker_worklist2[TIMER_LIST_SIZE];

    /* <128 milli-second */
    lm_timer_res_t *timer_worklist3[TIMER_LIST_SIZE];
    lm_timer_res_t *ticker_worklist3[TIMER_LIST_SIZE];

    /* >= 128 milli-second */
    lm_timer_res_t *timer_worklist4[TIMER_LIST_SIZE];
    lm_timer_res_t *ticker_worklist4[TIMER_LIST_SIZE];

    lm_msglist_t pubmsg_list[128];
    lm_msglist_t submsg_list[128];


};
typedef struct lmice_resource_parameter_s lm_res_param_t;


void forceinline get_timer_res_list(lm_res_param_t* pm, lm_timer_res_t* val, lm_timer_res_t*** tlist)
{
    lm_timer_info_t *info = val->info;
    if(info->type = TIMER_TYPE)
    {
        if(info->due > 0)
        {
            *tlist = pm->timer_duelist;
        }
        else
        {
            if(info->period < PERIOD_1)
                *tlist = pm->timer_worklist1;
            else if(info->period < PERIOD_2)
                *tlist = pm->timer_worklist2;
            else if(info->period < PERIOD_3)
                *tlist = pm->timer_worklist3;
            else
                *tlist = pm->timer_worklist4;
        }
    }
    else
    {
        if(info->due > 0)
        {
            *tlist = pm->ticker_duelist;
        }
        else
        {
            if(info->period < PERIOD_1)
                *tlist = pm->ticker_worklist1;
            else if(info->period < PERIOD_2)
                *tlist = pm->ticker_worklist2;
            else if(info->period < PERIOD_3)
                *tlist = pm->ticker_worklist3;
            else
                *tlist = pm->ticker_worklist4;
        }
    }
}

int forceinline remove_timer_from_tmlist(lm_res_param_t *pm, lm_timer_res_t* val)
{
    UNREFERENCED_PARAM(pm);
    val->active = 0;
    return 0;
//    int64_t pos = 0;
//    int ret = 1;
//    lm_timer_res_t *res = NULL;
//    lm_timer_res_t* cur = NULL;
//    lm_timer_res_t* tlist = NULL;

//    get_timer_res_list(pm, val, &tlist);
//    do
//    {
//        cur = tlist;
//        for(pos = 0; pos < tlist[TIMER_LIST_NEXT_POS].active; ++pos)
//        {
//            res = cur+pos;
//            if(res->info == val->info)
//            {
//                --tlist[TIMER_LIST_NEXT_POS].active;
//                memmove(res, res+1, (tlist[TIMER_LIST_NEXT_POS].active - pos)*sizeof(lm_timer_res_t) );
//                memset(cur+tlist[TIMER_LIST_NEXT_POS].active, 0, sizeof(lm_timer_res_t));
//                ret = 0;
//                break;
//            }
//        }
//        if(ret == 0)
//            break;
//        res = cur+TIMER_LIST_NEXT_POS;
//        cur=(lm_timer_res_t*) res->info;
//    } while(cur != NULL);

//    return ret;

}

int forceinline append_timer_to_tmlist(lm_res_param_t *pm, lm_timer_res_t* val)
{

    lm_timer_res_t **res = NULL;
    lm_timer_res_t **cur = NULL;
    lm_timer_res_t **tlist = NULL;

    get_timer_res_list(pm, val, &tlist);
    do
    {
        cur = tlist;
        if(tlist[TIMER_LIST_NEXT_POS]->active < TIMER_LIST_NEXT_POS)
        {
            /* append timer */
            res = &tlist[ tlist[TIMER_LIST_NEXT_POS]->active ];
            *res = val;
            ++ tlist[TIMER_LIST_NEXT_POS]->active;
            break;
        }

        tlist = (lm_timer_res_t**) tlist[TIMER_LIST_NEXT_POS]->info;
        if(tlist == NULL)
        {
            /* create a new pointer-array */
            tlist = (lm_timer_res_t**)malloc(sizeof(lm_timer_res_t*)*TIMER_LIST_SIZE);
            memset(tlist, 0, sizeof(lm_timer_res_t*)*TIMER_LIST_SIZE);

            /* create the last pointer element (for control) */
            tlist[TIMER_LIST_NEXT_POS]= (lm_timer_res_t*)malloc(sizeof(lm_timer_res_t));

            /* reset control element value */
            memset(tlist[TIMER_LIST_NEXT_POS], 0, sizeof(lm_timer_res_t) );

            /* append timer */
            res = &tlist[ tlist[TIMER_LIST_NEXT_POS]->active ];
            *res = val;
            ++ tlist[TIMER_LIST_NEXT_POS]->active;

            /* link newlist to current list */
            cur[TIMER_LIST_NEXT_POS]->info = (lm_timer_info_t*)((void *)tlist);
            cur[TIMER_LIST_NEXT_POS]->alist =(lm_ref_act_t*)((void *)tlist);

            break;
        }
    } while(tlist != NULL);

    return 0;

}


int create_resource_service(lm_res_param_t* pm);
int destroy_resource_service(lm_res_param_t* pm);

#endif /** RESOURCE_MANAGE_H */

