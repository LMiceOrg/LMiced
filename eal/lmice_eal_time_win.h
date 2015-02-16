#ifndef LMICE_EAL_TIME_WIN_H
#define LMICE_EAL_TIME_WIN_H

#include "lmice_eal_common.h"
#include <time.h>

void forceinline usleep(int usec){
    LARGE_INTEGER litmp;
    LONGLONG QPart1, QPart2;
    double dfMinus, dfFreq, dfTim;
    QueryPerformanceFrequency(&litmp);
    dfFreq = (double)litmp.QuadPart;

    QueryPerformanceCounter(&litmp);
    QPart1 = litmp.QuadPart;

    do {
        QueryPerformanceCounter(&litmp);
        QPart2 = litmp.QuadPart;

        dfMinus = (double)(QPart2-QPart1);
        dfTim = dfMinus / dfFreq;

    }while(dfTim<0.000001*usec);
}

int forceinline ctime_r(const time_t* tm, char* time)
{
    return ctime_s(time, 26, tm);
}

#endif /** LMICE_EAL_TIME_WIN_H */

