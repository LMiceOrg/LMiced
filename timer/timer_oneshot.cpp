#include "timer_system_time.h"
#include "eal/lmice_eal_common.h"

//#include <sglib.h>
#include <errno.h>
#include <eal/lmice_eal_spinlock.h>
#include "eal/lmice_eal_hash.h"
/**
 * @brief The lm_tick_timer_s struct 仿真时间定时器
 */
struct lm_tick_timer_s
{
    volatile int32_t state;         // 状态
    int32_t size;                   // 触发计数量
    HANDLE  event;                  // 响应事件
    int64_t instid;                 // 场景编号
    uint64_t timerid;                // 定时器编号
    int64_t tick;                   // 周期

    int64_t count;                  // 已完成触发数量
    int64_t begin_tick;             // 开始时间


};

enum lm_tick_timer_e
{
    TICK_TIMER_EMPTY = 1,
    TICK_TIMER_WORK = 2,
    TICK_TIMER_INFINITY = -1,
    TICK_TIMER_NOW    = 0
};

//Declaration
#define MAX_TICK_TIMER_SIZE 128
struct lm_tick_timer_s lm_tick_timer[MAX_TICK_TIMER_SIZE];
uint64_t lm_tick_timer_lock = 0;

void static forceinline generate_event_name(uint64_t hval, char* ename, size_t esize)
{
    const char* prefix = "Global\\";
    const char* num="0123456789ABCDEF";
    if(esize != 24)
        return; /*ENAME太小*/

    memcpy(ename, prefix, 7);
    for(int i=0; i<8; ++i )
    {
        ename[i*2+7] =num[ ( (unsigned char*)&hval )[i] >> 4 ];
        ename[i*2+7+1] = num[ ( (unsigned char*)&hval )[i] % 0x10 ];
    }

    ename[23] = 0;

}

void process_tick_timer()
{

    size_t id;
    int ret;
    int64_t now = tick_time_read();

    ret = eal_spin_trylock(&lm_tick_timer_lock);
    if(ret != 0)
        return;

    for(id = 0; id < sizeof(lm_tick_timer)/sizeof(struct lm_tick_timer_s); ++id )
    {
        struct lm_tick_timer_s *pt;
        pt = &lm_tick_timer[id];

        if(pt->state == TICK_TIMER_WORK)
        {
            /* 判断时间中断的仿真时间 */
            if( pt->begin_tick + pt->tick < now)
                continue;

            /* 判断响应次数 */
            if(pt->size == TICK_TIMER_INFINITY          /* 无限次数 */
                    || pt->count < pt->size)    /* 响应数小于请求数 */
            {
                ++pt->count;
                pt->begin_tick = now;
                SetEvent(pt->event);
            }
            else if(pt->count >= pt->size)      /* 响应数大于等于请求数 */
            {
                pt->state = TICK_TIMER_EMPTY;
                CloseHandle(pt->event);
                pt->event = NULL;
            }
        }
    }

    eal_spin_unlock(&lm_tick_timer_lock);
}

/**
 * @brief create_tick_timer
 * @param instid
 * @param tick
 * @param size
 * @param begin_tick
 * @param event
 * @param timer_id
 * @return
 */
int create_tick_timer(int64_t instid, int tick, int size,  int64_t begin_tick, HANDLE* event, uint32_t* timer_id)
{
    int ret;
    uint32_t id;
    uint64_t hval=0;
    char ename[24];

    ret = eal_spin_trylock(&lm_tick_timer_lock);
    if(ret != 0)
        return EBUSY;

    ret = EADDRINUSE;
    for(id=0; id< sizeof(lm_tick_timer)/sizeof(struct lm_tick_timer_s); ++id )
    {
        struct lm_tick_timer_s *pt;
        pt = &lm_tick_timer[id];
        if(pt->state == TICK_TIMER_EMPTY)
        {
            int64_t now = tick_time_read();
            if(begin_tick  == TICK_TIMER_NOW)
                begin_tick = now;
            else if (begin_tick < now )
                    begin_tick = now;

            hval = eal_hash64_fnv1a(&instid, sizeof(instid));
            hval = eal_hash64_more_fnv1a(&tick, sizeof(tick), hval);
            hval = eal_hash64_more_fnv1a(&size, sizeof(size), hval);
            hval = eal_hash64_more_fnv1a(&begin_tick, sizeof(begin_tick), hval);
            generate_event_name(hval, ename, sizeof(ename));
            *event = CreateEventA(NULL, FALSE, FALSE, ename);
            if(*event == NULL)
            {
                ret = EINVAL;
                break;
            }

            pt->state = TICK_TIMER_WORK;
            pt->size = size;
            pt->event = *event;
            pt->instid = instid;
            pt->timerid = id;
            pt->tick = tick;
            pt->count = 0;
            pt->begin_tick = begin_tick;

            *timer_id = id;
            ret = 0;
            break;
        }
    }


    eal_spin_unlock(&lm_tick_timer_lock);

    return ret;
}

/**
 * @brief delete_timer_oneshot
 * @param timer_id
 * @return 0- Success, else failed.
 */
int delete_tick_timer(uint32_t tc_id)
{
    int ret;
    struct lm_tick_timer_s *pt;

    if(tc_id >= sizeof(lm_tick_timer)/sizeof(struct lm_tick_timer_s))
        return ERANGE;

    ret = eal_spin_trylock(&lm_tick_timer_lock);
    if(ret != 0)
        return EBUSY;

    pt = &lm_tick_timer[tc_id];
    if(pt->state == TICK_TIMER_WORK)
    {
        pt->state = TICK_TIMER_EMPTY;
        CloseHandle(pt->event);
        pt->event = NULL;
    }

    eal_spin_unlock(&lm_tick_timer_lock);

    return 0;
}
