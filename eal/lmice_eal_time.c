#include "lmice_eal_time.h"

#include <stdlib.h>

#if defined(__MACH__)

void* eal_timer_thread(void * data)
{

    lm_timer_ctx_t *ctx = (lm_timer_ctx_t*)data;

    uint64_t factor = 1;
    uint64_t time_to_wait = 0;
    uint64_t now = 0;
    uint64_t interval = ctx->interval;

    void (*handler) (void*) = ctx->handler;
    volatile int64_t *quit_flag = ctx->quit_flag;
    void* context = ctx->context;

    /* clean data */
    eal_timer_free_context(ctx);

    /* init factor  of nano second */
    eal_init_timei(&factor);

    /* from 100x nano-seconds to nano-seconds */
    time_to_wait = interval * 100llu * factor;
    interval = time_to_wait;

    /* for-loop to work */
    for(;;) {
        if(*quit_flag == 1)
            break;

        now = mach_absolute_time() + time_to_wait;
        mach_wait_until(now);

        /* call user function */
        handler(context);

        time_to_wait = now + interval - mach_absolute_time();
        if(time_to_wait > interval)
            time_to_wait = 0;
    }

    return 0;
}

#endif
