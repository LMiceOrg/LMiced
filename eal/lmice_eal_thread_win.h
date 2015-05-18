#ifndef LMICE_EAL_THREAD_WIN_H
#define LMICE_EAL_THREAD_WIN_H

#include "lmice_eal_common.h"

forceinline eal_pid_t eal_gettid(void)
{
    return GetCurrentThreadId();
}

forceinline eal_pid_t eal_thisthread_id(void)
{
    return GetCurrentThreadId();
}

int forceinline pthread_getname_np(DWORD tid, char* name, size_t sz)
{
    UNREFERENCED_PARAM(tid);
    UNREFERENCED_PARAM(name);
    UNREFERENCED_PARAM(sz);
    /* No implementation*/
    return -1;
}

int forceinline pthread_setname_np(const char* name)
{
    UNREFERENCED_PARAM(name);
    return 0;
}

eal_pid_t forceinline getpid(void)
{
    return GetCurrentProcessId();
}

typedef void(*eal_thread_callback)(void*);
struct lmice_thread_context_s
{
    /* =1 quit */
    volatile int64_t* quit_flag;
    /* callback function pointer */
    void (*handler) (void*);
    /* callback parameter */
    void* context;
};
typedef struct lmice_thread_context_s lm_thread_ctx_t;

#define eal_thread_malloc_context(ctx) \
    ctx = (lm_thread_ctx_t*)malloc(sizeof(lm_thread_ctx_t));

#define eal_thread_free_context(ctx) \
    free(ctx); \

forceinline int eal_thread_create(HANDLE* thread, lm_thread_ctx_t* ctx)
{
    *thread = CreateThread(NULL, 0, eal_thread_thread, ctx, 0, NULL);
    if(*thread)
        return 0;
    return 1;
}

#endif /** LMICE_EAL_THREAD_WIN_H */

