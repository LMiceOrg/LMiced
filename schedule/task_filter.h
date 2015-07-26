#ifndef TASK_FILTER_H
#define TASK_FILTER_H

#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_hash.h"
#include "eal/lmice_eal_spinlock.h"
#include "task.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


/* Process mode */
#define LMICE_TASK_PROC_PASS 0
#define LMICE_TASK_PROC_TERMINATE 1

/* Process direction */
#define LMICE_TASK_PROC_IN 0
#define LMICE_TASK_PROC_OUT 1

/* Process level */
#define LMICE_TASK_PROC_LEVEL0 0
#define LMICE_TASK_PROC_LEVEL1 1
#define LMICE_TASK_PROC_LEVEL2 2
#define LMICE_TASK_PROC_LEVEL3 3


typedef int (* lmice_event_callback)(void* ctx, void* data);


/* Filter condition */
struct lmice_filter_condition_s
{
    uint64_t flt;       /* default filter */
    lmtsk_id_t tid;     /* task condition */
};
typedef struct lmice_filter_condition_s lmflt_cond_t;

/* predefined filter */
#define LMICE_FILTER_DATA_BIG_ENDIAN 1

/* Task filter */
struct lmice_task_filter_s
{
    uint64_t id;                /* filter identity */
    lmflt_cond_t cond;          /* filter condition */
    void*   pdata;              /* task data context */
    lmice_event_callback cb;   /* callback procedure */
};
typedef struct lmice_task_filter_s lmflt_t;

#define LMICE_FILTER_LIST_SIZE 63
/* task filter list */
struct lmice_task_filter_list_s
{
    volatile int64_t lock;      /* lock */
    int32_t pt;                 /* process type */
    int32_t pd;                 /* process direction */
    int32_t level;              /* process level */
    uint32_t size;              /* filter list size */
    struct lmice_task_filter_list_s * next; /* next filter pointer */
    lmflt_t flt_list[LMICE_FILTER_LIST_SIZE]; /* filter list */
};
typedef struct lmice_task_filter_list_s lmflt_list_t;

/* Dir(2) X Mode(2) X Level(4) */
#define TASK_FILTER_LIST_SIZE 16

#define LMICE_GET_FILTER_LIST(list, pdir, pmode, plevel) \
    &list[pdir + (pmode<<1) + (plevel<<2)]

forceinline int lmflt_append_task_filter(lmflt_list_t* list, int pdir, int pmode, int plevel, lmflt_cond_t* cond, void* pdata, lmice_event_callback cb, uint64_t* id) {
    lmflt_t* filter = NULL;
    lmflt_list_t* flt_list = NULL;
    volatile int64_t* lock = &flt_list->lock;

    /* Select filter list */
    flt_list = LMICE_GET_FILTER_LIST(list, pdir, pmode, plevel);

    /* calculate filter identity */
    *id = eal_hash64_fnv1a(cond, sizeof(lmflt_cond_t));
    *id = eal_hash64_more_fnv1a(&pdata, sizeof(pdata), *id);
    *id = eal_hash64_more_fnv1a(&cb, sizeof(cb), *id);

    eal_spin_lock(lock);

    /* loop to append filter */
    do {
        if(flt_list->size < LMICE_FILTER_LIST_SIZE ) {

            filter = flt_list->flt_list + flt_list->size;
            filter->id = *id;
            filter->cb = cb;
            memcpy(&filter->cond, cond, sizeof(lmflt_cond_t));
            filter->pdata = pdata;

            ++flt_list->size;
            break;
        } else {
            if(flt_list->next == NULL) {
                flt_list->next = (lmflt_list_t*)malloc(sizeof(lmflt_list_t));
                memset(flt_list->next, 0, sizeof(lmflt_list_t));
                flt_list->next->pd = flt_list->pd;
                flt_list->next->pt = flt_list->pt;
                flt_list->next->level = flt_list->level;
            }
            flt_list = flt_list->next;
        }
    } while(flt_list != NULL);

    eal_spin_unlock(lock);

    return 0;
}

forceinline int lmflt_remove_task_filter(lmflt_list_t* list, int pdir, int pmode, int plevel, uint64_t id){
    lmflt_t* filter = NULL;
    lmflt_list_t* flt_list = NULL;
    uint32_t i = 0;
    int remove_flag = 1;
    volatile int64_t* lock = &flt_list->lock;

    /* Select filter list */
    flt_list = LMICE_GET_FILTER_LIST(list, pdir, pmode, plevel);

    eal_spin_lock(lock);

    /* loop to remove filter */
    do {
        for(i=0; i < flt_list->size; ++i ) {

            filter = flt_list->flt_list + i;
            if(filter->id == id) {
                memmove(filter, filter+1, (flt_list->size - i - 1)*sizeof(lmflt_t) );
                memset(flt_list->flt_list+flt_list->size -1, 0, sizeof(lmflt_t) );
                --flt_list->size;
                remove_flag = 0;
                break;
            }
        }
        if(remove_flag == 0) {
            break; /* break-do */
        }

        /* Search next filter list */
        flt_list = flt_list->next;

    } while(flt_list != NULL);

    eal_spin_unlock(lock);

    return remove_flag;
}

int lmflt_subscribe_type(void* ptask, void* pdata);

int lmflt_subscribe_object(void* ptask, void* pdata);

#endif /** TASK_FILTER_H */
