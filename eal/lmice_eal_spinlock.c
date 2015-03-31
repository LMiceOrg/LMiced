#include "lmice_eal_spinlock.h"

#include "lmice_eal_atomic.h"
#include "lmice_eal_time.h"

#define LOCK_LOOP_COUNT         20000000LL

int eal_spin_trylock(uint64_t* lock)
{
    int ret = 1;
    uint64_t cnt = 0;
    uint64_t locked = 1;
    int sleep_times = 0;

    do {
        locked = eal_compare_and_swap64(lock, 0, 1);
        if(locked == 1)
        {
            ret = 0;
            break;
        }
        cnt ++;
        if(cnt == LOCK_LOOP_COUNT)
        {
            usleep(1);
            sleep_times++;
            cnt = 0;
        }
    }while(sleep_times < 10);

    return ret;
}

int eal_spin_lock(uint64_t* lock)
{
    uint64_t locked = 1;
    while(locked == 0)
    {
        locked = eal_compare_and_swap64(lock, 0, 1);
    }
    return 0;
}

int eal_spin_unlock(uint64_t* lock)
{
    eal_fetch_and_sub64(lock, 1);
    return 0;
}
