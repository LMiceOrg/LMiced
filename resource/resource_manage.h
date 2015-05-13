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
#include "eal/lmice_trace.h"


#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define PUBLISH_RESOURCE_TYPE  1
#define SUBSCRIBE_RESOURCE_TYPE  2
#define TICKER_TYPE 1
#define TIMER_TYPE 2

#define CLIENT_SHMNAME  "CC597303-0F85-40B2-8BDC-4724BD" /** C87B4E */
#define BOARD_SHMNAME   "82E0EE49-382C-40E7-AEA2-495999" /** 392D29 */
#define DEFAULT_SHM_SIZE 4096 /** 4KB */
#define DEFAULT_CLIENT_SIZE 200
#define DEFAULT_RESOURCE_SIZE 128
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

enum lmice_timer_state_e
{
    LM_TIMER_NOTUSE = 0,
    LM_TIMER_RUNNING = 2,
    LM_TIMER_DELETE = 1

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

struct forcepack(8) lmice_state_s
{
    uint8_t value[8];
};
typedef struct lmice_state_s lm_state_t;

struct lmice_shm_resourece_s
{
    evtfd_t efd;
    shmfd_t sfd;
    int32_t padding0;
    addr_t  addr;
};
typedef struct lmice_shm_resourece_s lm_shm_res_t;


/**
 * @brief The lmice_action_info_s struct
 * 用户事件描述
 */
struct lmice_action_info_s
{
    uint32_t type;              /* join */
    uint32_t size;
    uint64_t inst_id;           /* 实例编号 */
    uint64_t act_ids[8];
    lm_state_t state;

};
typedef struct lmice_action_info_s lm_action_info_t;


struct lmice_referenced_acion_s
{
    uint32_t pos;
    int32_t padding0;
    lm_action_info_t                *info;
    struct lmice_referenced_acion_s *next;
};
typedef struct lmice_referenced_acion_s lm_ref_act_t;

struct lmice_action_res_s
{
    int active;
    int padding0;
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
    uint64_t count;             /* 已完成触发数量 */
    int64_t  begin;             /* 开始时间 */
    lm_state_t state;
};
typedef struct lmice_timer_s lm_timer_t;

/**
 * @brief The lmice_timer_info_s struct
 * 定时器描述,同时包括定时器运行时状态
 */
struct lmice_timer_info_s
{
    uint32_t type;              /* ticker timer */
    uint32_t size;              /* 触发计数量 1 --> one-shot  0 --> infinite */
    int64_t period;             /* 周期长度 */
    int64_t due;                /* 预期开始时间, -1 立即开始, 0 下周期开始 */
    uint64_t inst_id;           /* 实例编号 */
    lm_timer_t  timer;          /* 定时器状态 */
};
typedef struct lmice_timer_info_s lm_timer_info_t;

struct lmice_worker_resource_s;
struct lmice_timer_resource_s
{
    uint64_t active;
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
    volatile int32_t lock;  /* sync purpose */
    uint32_t size;          /* blob size( bytes) */
    char blob[8];
};
typedef struct lmice_message_s lm_mesg_t;

/**
 * @brief The lmice_message_info_s struct
 * 消息描述
 */
struct lmice_message_info_s
{
    uint32_t        type;       /* publish; subscribe by instance or subscribe by type */
    uint32_t        size;       /* size */
    int64_t         period;     /* message publish  period tick, zero(0) means no period */
    uint64_t inst_id;           /* 实例编号 (决定了 响应事件与共享内存编号) */
    uint64_t type_id;           /* 类型编号 */
    lm_state_t state;           /* 状态 */
};
typedef struct lmice_message_info_s lm_mesg_info_t;

/**
 * @brief The lmice_message_resource_s struct
 * 消息资源
 */
struct lmice_message_resource_s
{
    /*int64_t active; */
    addr_t addr;
    shmfd_t sfd;
    int32_t padding0;
    /*    lm_shm_res_t    res; */
    lm_mesg_info_t  *info;
    lm_ref_act_t    *alist;
    uint64_t        worker_id;
};
typedef struct lmice_message_resource_s lm_mesg_res_t;

struct lmice_event_info_s
{
    uint64_t inst_id;
    uint64_t value;
};

typedef struct lmice_event_info_s lm_evt_info_t;

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

    volatile int64_t lock;
    uint64_t inst_id;   /* 实例编号 */
    uint64_t type_id;   /* 类型编号 */

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
    int32_t process_id;
#if !defined(_WIN32)
    int32_t padding0;
#endif
    /* worker thread id, if equal to zero(0) means process-level instance */
    eal_tid_t thread_id;
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
    volatile int64_t lock;
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
    lm_worker_res_t     *res_worker;    /* worker pos */
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
    HANDLE      timer;
    volatile int64_t quit_flag;

};
#elif defined(__APPLE__) || defined(__BSD__)

/* GCD(Grand Central Dispatch)
 *
 * Apple 10.6(Snow Leopand)
 * FreeBSD 8.1
 *
*/
#include <dispatch/dispatch.h>
struct lmice_time_parameter_s
{
    lm_time_t * pt;
    pthread_t timer;
    dispatch_source_t wTimerID;
    dispatch_queue_t queue;
    uint64_t wTimerRes;
    uint64_t wTimerDelay;
    uint64_t count;
    volatile int64_t quit_flag;
};

#elif defined(__LINUX__)

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
#define TIMER_LIST_NEXT_POS 0
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

    /* due list */
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

    /* 完成端口 */
    evtfd_t cp;
};
typedef struct lmice_resource_parameter_s lm_res_param_t;


forceinline void get_timer_res_list(lm_res_param_t* pm, lm_timer_res_t* val, lm_timer_res_t*** tlist)
{
    lm_timer_info_t *info = val->info;
    if(info->type == TIMER_TYPE)
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


forceinline void append_timer_to_tlist(lm_timer_res_t *val, lm_timer_res_t **tlist)
{
    lm_timer_res_t **res = NULL;
    lm_timer_res_t **cur = NULL;

    do
    {
        cur = tlist;
        if(tlist[TIMER_LIST_NEXT_POS]->active < TIMER_LIST_SIZE - 1)
        {
            /* append timer */
            res = &tlist[ ++ tlist[TIMER_LIST_NEXT_POS]->active ];
            *res = val;
            /* lmice_debug_print("append timer[%lld] at[%lld] [%p]\n", val->active, tlist[TIMER_LIST_NEXT_POS]->active, *tlist);
             */
            break;
        }

        tlist = (lm_timer_res_t**) tlist[TIMER_LIST_NEXT_POS]->info;
        if(tlist == NULL)
        {
            /* create a new pointer-array */
            tlist = (lm_timer_res_t**)malloc(sizeof(lm_timer_res_t*)*TIMER_LIST_SIZE);
            memset(tlist, 0, sizeof(lm_timer_res_t*)*TIMER_LIST_SIZE);

            /* create the next-pos element (for control) */
            tlist[TIMER_LIST_NEXT_POS]= (lm_timer_res_t*)malloc(sizeof(lm_timer_res_t));

            /* reset control element value */
            memset(tlist[TIMER_LIST_NEXT_POS], 0, sizeof(lm_timer_res_t) );

            /* append timer */
            res = &tlist[ ++ tlist[TIMER_LIST_NEXT_POS]->active ];
            *res = val;


            /* link new array to current array */
            cur[TIMER_LIST_NEXT_POS]->info = (lm_timer_info_t*)((void *)tlist);
            cur[TIMER_LIST_NEXT_POS]->alist =(lm_ref_act_t*)((void *)tlist);

            break;
        }
    } while(tlist != NULL);
}

forceinline int remove_timer_from_list_by_worker(lm_timer_res_t **tlist, lm_worker_res_t* val)
{
    uint64_t pos = 0;
    lm_timer_res_t *res = NULL;
    do
    {
        for(pos = 1; pos <= tlist[TIMER_LIST_NEXT_POS]->active; ++pos)
        {
            res = tlist[pos];

            if(res->worker == val)
            {
                memmove(tlist+pos, tlist+pos+1, (tlist[TIMER_LIST_NEXT_POS]->active - pos)*sizeof(lm_timer_res_t*) );
                tlist[ tlist[TIMER_LIST_NEXT_POS]->active ] = NULL;
                --tlist[TIMER_LIST_NEXT_POS]->active;
            }

        } /* end-for */

        /* get next array */
        tlist = (lm_timer_res_t **)tlist[TIMER_LIST_NEXT_POS]->info;
    } while(tlist != NULL);

    return 0;
}

forceinline int remove_timer_by_worker(lm_res_param_t *pm, lm_worker_res_t* val)
{
    int ret = 0;
    lm_timer_res_t **tlist = NULL;

    tlist = pm->timer_duelist;
    ret = remove_timer_from_list_by_worker(tlist, val);


    tlist = pm->timer_worklist1;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->timer_worklist2;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->timer_worklist3;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->timer_worklist4;
    ret = remove_timer_from_list_by_worker(tlist, val);


    tlist = pm->ticker_duelist;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->ticker_worklist1;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->ticker_worklist2;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->ticker_worklist3;
    ret = remove_timer_from_list_by_worker(tlist, val);

    tlist = pm->ticker_worklist4;
    ret = remove_timer_from_list_by_worker(tlist, val);


    return ret;
}

forceinline int remove_timer_from_list(lm_timer_res_t **tlist, lm_timer_res_t* val)
{
    uint64_t pos = 0;
    lm_timer_res_t *res = NULL;
    do
    {
        for(pos = 1; pos <= tlist[TIMER_LIST_NEXT_POS]->active; ++pos)
        {
            res = tlist[pos];

            if(res == val)
            {
                lmice_error_print("remove_timer_from_list %p\n",  res);
                res->active = LM_TIMER_NOTUSE;
                memmove(tlist+pos, tlist+pos+1, (tlist[TIMER_LIST_NEXT_POS]->active - pos)*sizeof(lm_timer_res_t*) );
                tlist[ tlist[TIMER_LIST_NEXT_POS]->active ] = NULL;
                --tlist[TIMER_LIST_NEXT_POS]->active;
                return 0;
            }

        } /* end-for */

        /* get next array */
        tlist = (lm_timer_res_t **)tlist[TIMER_LIST_NEXT_POS]->info;
    } while(tlist != NULL);

    return 1;
}

forceinline int remove_timer_from_res(lm_res_param_t *pm, lm_timer_res_t* val)
{
    int ret = 0;
    lm_timer_res_t **tlist = NULL;
    if(val->info->type == TIMER_TYPE)
    {
        tlist = pm->timer_duelist;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;

        tlist = pm->timer_worklist1;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;
        tlist = pm->timer_worklist2;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;
        tlist = pm->timer_worklist3;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;

        tlist = pm->timer_worklist4;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;

    }
    else
    {
        tlist = pm->ticker_duelist;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;

        tlist = pm->ticker_worklist1;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;
        tlist = pm->ticker_worklist2;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;
        tlist = pm->ticker_worklist3;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;

        tlist = pm->ticker_worklist4;
        ret = remove_timer_from_list(tlist, val);
        if(ret == 0)
            return 0;
    }

    return ret;
}

forceinline int append_timer_to_res(lm_res_param_t *pm, lm_timer_res_t* val)
{
    lm_timer_res_t **tlist = NULL;

    get_timer_res_list(pm, val, &tlist);
    append_timer_to_tlist(val, tlist);



    return 0;

}

enum lmice_resource_task_type_e
{
    LM_RES_TASK_NOTUSE = 0,
    LM_RES_TASK_ADD_TIMER,
    LM_RES_TASK_DEL_TIMER,
    LM_RES_TASK_ADD_ACTION,
    LM_RES_TASK_DEL_ACTION,
    LM_RES_TASK_ADD_PUBMSG,
    LM_RES_TASK_DEL_PUBMSG,
    LM_RES_TASK_ADD_SUBMSG,
    LM_RES_TASK_DEL_SUBMSG,
    LM_RES_TASK_ADD_WORKER,
    LM_RES_TASK_DEL_WORKER
};

struct lmice_resource_task_s
{
    int type;           /* 任务类型 */
    int32_t padding0;
    uint64_t inst_id;   /* 资源ID */
    void* pval;         /* 指向资源地址 */
};
typedef struct lmice_resource_task_s lm_res_task_t;

int peek_resource_task(lm_res_task_t* task);
int set_resource_task(lm_res_task_t* task);

int append_worker_to_res(lm_res_param_t* pm, lm_worker_res_t* worker);
int remove_worker_from_res(lm_res_param_t* pm, lm_worker_res_t* worker);

forceinline int resource_task_proc(lm_res_param_t* pm)
{
    lm_res_task_t task;
    int ret = 0;

    for(;;)
    {
        memset(&task, 0, sizeof(lm_res_task_t));
        ret = peek_resource_task(&task);
        if(ret != 0)
            return 0;
        if(task.type == LM_RES_TASK_NOTUSE)
            return 0;


        switch(task.type)
        {
        case LM_RES_TASK_NOTUSE:
            break;
        case LM_RES_TASK_ADD_TIMER:
            ret = append_timer_to_res(pm, (lm_timer_res_t*)task.pval);
            break;
        case LM_RES_TASK_DEL_TIMER:
            ret = remove_timer_from_res(pm, (lm_timer_res_t*)task.pval);
            break;
        case LM_RES_TASK_ADD_WORKER:
            ret = append_worker_to_res(pm, (lm_worker_res_t*)task.pval);
            break;
        case LM_RES_TASK_DEL_WORKER:
            ret = remove_worker_from_res(pm, (lm_worker_res_t*)task.pval);
            break;
        }
    }
}

int create_resource_service(lm_res_param_t* pm);
int destroy_resource_service(lm_res_param_t* pm);

#endif /** RESOURCE_MANAGE_H */

