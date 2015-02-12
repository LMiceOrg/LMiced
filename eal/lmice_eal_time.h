#ifndef LMICE_EAL_TIME_H
#define LMICE_EAL_TIME_H

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include "lmice_eal_time_win.h"
#else
#error("No time implementation!")
#endif

#endif /** LMICE_EAL_TIME_H */

