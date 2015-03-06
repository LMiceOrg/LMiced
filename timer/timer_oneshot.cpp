#include <sglib.h>
#include <errno.h>
#include <eal/lmice_eal_spinlock.h>
typedef void (timer_callback) (void* pdata);

/**
 * @brief The lm_timer_s struct 定时器
 */
struct lm_timer_s
{
    int32_t state;          // 状态
    int32_t size;           // 触发计数量
    timer_callback tc;      // 触发回调函数
    void* pdata;            // 回调函数参数

    int64_t count;          // 已完成触发数量
    int64_t begin_time;     // 开始时间
    int64_t tick;           // 周期
    int64_t instid;         // 场景编号
    int64_t timerid;        // 定时器编号
};

enum os_timer_e
{
    OS_TIMER_EMPTY,
    OS_TIMER_WORK
};

//Declaration
struct lm_timer_s lm_timer[128];
int64_t lock_lm_timer = 0;

void process_timer_onshot()
{

    int id;
    for(id = 0; id < sizeof(lm_timer)/sizeof(struct lm_timer_s); ++id )
    {
        struct lm_timer_s *pt;
        pt = &os_timer[id];
        if(pt->state == OS_TIMER_WORK)
        {

        }
    }
}

/**
 * @brief create_timer_oneshot
 * @param tick_count
 * @param tc
 * @param pdata
 * @param timer_id
 * @return
 */
int create_timer_oneshot(int64_t instid, int tick, timer_callback tc, void* pdata, int* timer_id)
{
    int ret;
    int id;

    ret = eal_spin_trylock(&lock_lm_timer);
    if(ret != 0)
        return EBUSY;

    ret = EADDRINUSE;
    for(id=0; id< sizeof(lm_timer)/sizeof(struct lm_timer_s); ++id )
    {
        struct lm_timer_s *pt;
        pt = &os_timer[id];
        if(pt->state == OS_TIMER_EMPTY)
        {
            pt->instid = instid;
            pt->begin_time =0;
            pt->pdata = pdata;
            pt->state = OS_TIMER_WORK;
            pt->tc = tc;
            pt->tick = tick;
            *timer_id = id;

            ret = 0;
            break;
        }
    }


    eal_spin_unlock(&lock_lm_timer);

    return ret;
}

/**
 * @brief delete_timer_oneshot
 * @param timer_id
 * @return 0- Success, else failed.
 */
int delete_timer_oneshot(int timer_id)
{
    struct os_timer_s *pt;
    if(timer_id >= sizeof(os_timer)/sizeof(struct os_timer_s))
        return ERANGE;
    pt = os_timer[timer_id];
    pt->state = OS_TIMER_EMPTY;
    return 0;
}
