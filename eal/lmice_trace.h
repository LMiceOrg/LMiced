#ifndef TRACE_H
#define TRACE_H

#include "lmice_eal_common.h"
#include "lmice_eal_thread.h"
#include "lmice_eal_time.h"
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

#ifdef _WIN32
struct lmice_trace_name_s {
    lmice_trace_type_t type;
    char name[16];
    int color;
};
#define LMICE_TRACE_COLOR_TAG3(type) \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), lmice_trace_name[type].color); \
    printf(lmice_trace_name[type].name); \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), lmice_trace_name[lmice_trace_none].color);

#else
struct lmice_trace_name_s {
    lmice_trace_type_t type;
    char name[16];
    char color[16];
};

#define LMICE_TRACE_COLOR_TAG3(type) lmice_trace_name[type].color, lmice_trace_name[type].name, lmice_trace_name[lmice_trace_none].color

#endif
typedef struct lmice_trace_name_s lmice_trace_name_t;

extern const int lmice_trace_debug_mode;
extern lmice_trace_name_t lmice_trace_name[];

#if defined(_WIN32)

#define LMICE_TRACE_COLOR_PRINT(type, format, ...) do{ \
    char current_time[26];  \
    time_t tm;  \
    if(lmice_trace_debug_mode == 0 && type == lmice_trace_debug)  \
        break; \
    time(&tm);  \
    ctime_r(&tm, current_time); \
    /*change newline to space */    \
    current_time[24] = ' '; \
    printf(current_time); \
    LMICE_TRACE_COLOR_TAG3(type) \
    printf(":" format "\n", ##__VA_ARGS__); \
    }while (0);

#define LMICE_TRACE_COLOR_PRINT_PER_THREAD(type, format, ...) \
    do{ \
    int trace_ret_i;    \
    size_t trace_sz_i;  \
    time_t trace_tm_i;  \
    char trace_current_time_i[26];  \
    char trace_thread_name_i[32]; \
    if(lmice_trace_debug_mode == 0 && \
        type == lmice_trace_debug)  \
        break; \
    time(&trace_tm_i);  \
    ctime_r(&trace_tm_i, trace_current_time_i); \
    trace_current_time_i[24] = ' '; \
    trace_ret_i = pthread_getname_np(pthread_self(), trace_thread_name_i, 32);  \
    if(trace_ret_i == 0) {   \
        trace_sz_i = strlen(trace_thread_name_i);  \
        if(trace_sz_i == 0) trace_ret_i = -1;  \
        else trace_ret_i = 0;   \
    }   \
    printf(trace_current_time_i); \
    LMICE_TRACE_COLOR_TAG3(type); \
    if(trace_ret_i == 0) {   \
        printf(":[%d:%s]", getpid(), trace_thread_name_i); \
        printf(format , ##__VA_ARGS__); \
        printf("\n"); \
    } else {    \
        printf(":[%d:0x%x]", getpid(), pthread_self()); \
        printf(format , ##__VA_ARGS__); \
        printf("\n"); \
    }   \
    }while (0);


#else /** Posix */
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
        printf("%s%s%s%s:[%d:0x%x]" \
            format \
            "\n", \
            current_time, LMICE_TRACE_COLOR_TAG3(type), getpid(), pthread_self(), ##__VA_ARGS__); \
    }while (0);

#endif
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
        "%s**%s:thread[0x%x] -- %s[%d]"  format "\n", \
        current_time, lmice_trace_name[type].name, pthread_self(), __FILE__,__LINE__, ##__VA_ARGS__); \
    else    \
        fprintf((env)->logfd, \
        "%s**%s:thread[0x%x]"  format "\n", \
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
