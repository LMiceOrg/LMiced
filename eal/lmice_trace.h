#ifndef TRACE_H
#define TRACE_H

#include "lmice_eal_thread.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

enum lmice_trace_type_e
{
    lmice_trace_info        =0,
    lmice_trace_debug       =1,
    lmice_trace_warning     =2,
    lmice_trace_error       =3,
    lmice_trace_critical    =4,
    lmice_trace_none        =5
};

typedef enum lmice_trace_type_e lmice_trace_type_t;

struct lmice_trace_name_s {
    lmice_trace_type_t type;
    char name[16];
    char color[16];
};

typedef struct lmice_trace_name_s lmice_trace_name_t;

extern const int lmice_trace_debug_mode;
extern lmice_trace_name_t lmice_trace_name[];

#define LMICE_TRACE_COLOR_TAG3(type) lmice_trace_name[type].color, lmice_trace_name[type].name, lmice_trace_name[lmice_trace_none].color

#define LMICE_TRACE_COLOR_PRINT(type, format, ...) do{ \
    char current_time[26];  \
    time_t tm;  \
    if(lmice_trace_debug_mode == 0 && type == lmice_trace_debug)  \
        break; \
    time(&tm);  \
    ctime_r(&tm, current_time); \
    /*change newline to space */    \
    current_time[24] = ' '; \
    printf("%s%s%s%s:" format "\n", current_time, LMICE_TRACE_COLOR_TAG3(type), ##__VA_ARGS__); \
    }while (0);

#define LMICE_TRACE_COLOR_PRINT_PER_THREAD(type, format, ...) do{ \
    int ret;    \
    time_t tm;  \
    char current_time[26];  \
    char thread_name[32]; \
    if(lmice_trace_debug_mode == 0 && type == lmice_trace_debug)  \
        break; \
    time(&tm);  \
    ctime_r(&tm, current_time); \
    /*change newline to space */    \
    current_time[24] = ' '; \
    ret = pthread_getname_np(pthread_self(), thread_name, 32);  \
    if(ret == 0) {   \
        ret = strlen(thread_name);  \
        if(ret == 0) ret = -1;  \
        else ret = 0;   \
    }   \
    if(ret == 0)    \
        printf("%s%s%s%s:[%d:%s]" \
            format \
            "\n", \
            current_time, LMICE_TRACE_COLOR_TAG3(type), getpid(), thread_name, ##__VA_ARGS__); \
    else    \
        printf("%s%s%s%s:[%d:0x%tx]" \
            format \
            "\n", \
            current_time, LMICE_TRACE_COLOR_TAG3(type), getpid(), pthread_self(), ##__VA_ARGS__); \
    }while (0);

#define LMICE_TRACE_PER_THREAD(env, type, format, ...) do{ \
    char current_time[26];  \
    time_t tm;  \
    if(lmice_trace_debug_mode == 0 && type == lmice_trace_debug)  \
        break; \
    time(&tm);  \
    ctime_r(&tm, current_time); \
    /*change newline to space */    \
    current_time[24] = ' '; \
    if(lmice_trace_debug_mode == 1) \
        fprintf((env)->logfd, \
        "%s**%s:thread[0x%tx] -- %s[%d]"  format "\n", \
        current_time, lmice_trace_name[type].name, pthread_self(), __FILE__,__LINE__, ##__VA_ARGS__); \
    else    \
        fprintf((env)->logfd, \
        "%s**%s:thread[0x%tx]"  format "\n", \
        current_time, lmice_trace_name[type].name, pthread_self(), ##__VA_ARGS__); \
    }while(0);


#define LMICE_TRACE(env, type, format, ...) do { \
    char current_time[26];  \
    time_t tm;  \
    if(lmice_trace_debug_mode == 0 && type == lmice_trace_debug)  \
        break; \
    time(&tm);  \
    ctime_r(&tm, current_time); \
    /*change newline to space */    \
    current_time[24] = ' '; \
    if(lmice_trace_debug_mode == 1) \
        fprintf((env)->logfd, \
        "%s**%s: -- %s[%d]"  format "\n", \
        current_time, lmice_trace_name[type].name,  __FILE__,__LINE__, ##__VA_ARGS__); \
    else    \
        fprintf((env)->logfd, \
        "%s**%s:"  format "\n", \
        current_time, lmice_trace_name[type].name,  ##__VA_ARGS__); \
    } while(0);


#define lmice_info_print(format,...)      LMICE_TRACE_COLOR_PRINT_PER_THREAD(lmice_trace_info,        format, ##__VA_ARGS__)

#define lmice_debug_print(format,...)     LMICE_TRACE_COLOR_PRINT_PER_THREAD(lmice_trace_debug,       format, ##__VA_ARGS__)

#define lmice_warning_print(format,...)   LMICE_TRACE_COLOR_PRINT_PER_THREAD(lmice_trace_warning,     format, ##__VA_ARGS__)

#define lmice_error_print(format,...) LMICE_TRACE_COLOR_PRINT_PER_THREAD(lmice_trace_error,       format, ##__VA_ARGS__)

#define lmice_critical_print(format,...)  LMICE_TRACE_COLOR_PRINT_PER_THREAD(lmice_trace_critical,    format, ##__VA_ARGS__)

#define lmice_info_log(env, format,...) LMICE_TRACE_PER_THREAD(env, lmice_trace_info, format, ##__VA_ARGS__)
#define lmice_debug_log(env, format,...) LMICE_TRACE_PER_THREAD(env, lmice_trace_debug, format, ##__VA_ARGS__)
#define lmice_warning_log(env, format,...) LMICE_TRACE_PER_THREAD(env, lmice_trace_warning, format, ##__VA_ARGS__)
#define lmice_error_log(env, format,...) LMICE_TRACE_PER_THREAD(env, lmice_trace_error, format, ##__VA_ARGS__)
#define lmice_critical_log(env, format,...) LMICE_TRACE_PER_THREAD(env, lmice_trace_critical, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /** TRACE_H */
