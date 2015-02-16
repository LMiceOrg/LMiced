/** Project */
#include "system/system_environment_internal.h"
#include "system/system_environment.h"
#include "system/system_worker.h"
#include "system/system_master.h"
#include "system/system_signal.h"


/** Solution lib */
#include "lmice_trace.h"
#include "lmice_eal_daemon.h"

/** System */
//#include <arpa/inet.h>
//#include <pthread.h>
//#include <unistd.h>
//#include <sys/time.h>
#include <time.h>

/** C standard */
#include <signal.h>
#include <stdio.h>
#include <string.h>


/** C++ standard */
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <climits>
#include <cwchar>
#include <limits>


/*
author: jbenet
os x, compile with: gcc -o testo test.c
linux, compile with: gcc -o testo test.c -lrt
*/

#include <time.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h> /* mach_absolute_time */
#endif

/*
void* wait_time(void* tvar)
{
    int* pi = (int*)tvar;
    int i=0;
    mach_timebase_info_data_t tb;
    mach_timebase_info(&tb);
    uint64_t time_to_wait = (10000ULL * 100ULL)*tb.denom/tb.numer;
    uint64_t now[10], u64;
    struct timespec future[10];
    int timer[10] ={0};
    char name[32]={0};
    sprintf(name, "worker_%d", *pi);
    pthread_setname_np(name);
    for(i=0;i<10;++i)
    {
        now[i]  = mach_absolute_time();
        if(i==0)
            mach_wait_until(now[i] + time_to_wait);
        else
        {
            u64 = now[i] - now[i-1];
            if(time_to_wait*2 > u64)
                mach_wait_until(now[i]+ time_to_wait*2 - u64);

        }
        //        i++;
        //        if(i>= 5)
        //            break;
        //        timer[0] += 10;
        //        timer[1] += 100;
        //        for(int j=2; j<10; ++j)
        //            timer[j] ++;
    }
    for(int j=0; j<i; ++j)
    {
        future[j].tv_sec = now[j] /     1000000000LL;
        future[j].tv_nsec = now[j] % 1000000000UL;// /    1000000LL;
    }
    for(int j=1; j<i; ++j)
    {
        double d = (future[j].tv_nsec - future[j-1].tv_nsec);
        double rate = d / (double)time_to_wait;
        if(rate > 1.25 || rate < 0.75)
        {
            lmice_critical_print("time[%d] [%ld, %ld, %ld, %ld] [%ld]\t %d\n",
                                 j,
                                 future[j].tv_sec,
                                 future[j].tv_nsec / 1000000UL,
                                 (future[j].tv_nsec / 1000UL) % 1000,
                                 (future[j].tv_nsec) % 1000,
                                 (future[j].tv_nsec - future[j-1].tv_nsec),
                    timer[j]);
        }
        else
        {
            lmice_info_print("time[%d] [%ld, %ld, %ld, %ld] [%ld]\t %d\n",
                             j,
                             future[j].tv_sec,
                             future[j].tv_nsec / 1000000UL,
                             (future[j].tv_nsec / 1000UL) % 1000,
                             (future[j].tv_nsec) % 1000,
                             (future[j].tv_nsec - future[j-1].tv_nsec),
                    timer[j]);
        }
    }

    lmice_warning_print("total time:%ld\n",(future[9].tv_nsec - future[0].tv_nsec) );
    return nullptr;
}

*/

int main(int argc, char* argv[])
{
    int ret;
    using namespace lmice;
    SystemEnvironment lmice_env;
    SystemEnvironment* env = &lmice_env;
//    pthread_t thd[10];
//    int tvar[10];

//    uint64_t initclock = mach_absolute_time();
//    struct mach_timebase_info ti;
//    mach_timebase_info(&ti);
//    struct timespec future;
//    future.tv_sec = initclock /     1000000000LL;
//    future.tv_nsec = initclock % 1000000000UL;

//    lmice_debug_print("ts is %u, %u\t%llu\n", ti.numer, ti.denom, initclock);
//    lmice_warning_print("sec %ld, nsec %ld\n", future.tv_sec, future.tv_nsec)
//            ;
//    for(int i=0; i<10; ++i)
//    {
//        tvar[i] = i;
//        pthread_create(&thd[i], NULL, wait_time, &tvar[i]);
//    }

//    for(int i=0; i<10; ++i)
//    {
//        pthread_join(thd[i],NULL);
//    }

    ret = env->processCommandLine(argc, argv);
    if(ret != 0)
        return 0;

    lmice_debug_print("begin env init");
    ret = env->initialize();
    if(ret != 0)
        return 0;

    pthread_setname_np("LMicedMainThread");


    switch(env->role())
    {
    case 0://worker
    {
        SystemWorker worker(env);
        worker.processSignal();
    }
        break;
    case 1://master
    {
        SystemMaster master(env);
        master.CreateDaemon();

        if( (ret = master.initialize() ) != 0)
            return ret;

        ret = master.initSignal();
        if(ret != 0 )
            return ret;

        master.mainLoop();
    }
        break;
    case 2://cleaner
        lmice_debug_print("force clean");
        lmice_force_clean(env);
        break;
    default://unknown
        break;
    }


    return 0;
}

