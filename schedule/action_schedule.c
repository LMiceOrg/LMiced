
#include "action_schedule.h"
#include "timer/timer_system_time.h"

#include "eal/lmice_trace.h"
#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_atomic.h"
#include "eal/lmice_eal_endian.h"
#include "lmice_core.h"

#include "io/io_schedule.h"
#include "filter_tls.h"

#define LMICE_TASKLIST_EVENT_NAME "LMiced Tasklist Event"

/* Acquire a task from queue thread safe */
static int lmice_acquire_task(lmshd_ring_t* list, lmshd_task_t **task)
{
    size_t i=0;
    int ret = 1;
    int64_t val;
    lmshd_task_t* cur_task = NULL;
    uint64_t cur_pos = list->read_pos;
    for(i=0; i< TASKPROC_LIST_SIZE; ++i,++cur_pos) {

        cur_task = list->task + (cur_pos & (TASKPROC_LIST_SIZE-1) );
        val = eal_compare_and_swap64(&cur_task->lock, LM_RING_READ, LM_RING_READING);
        if(val == LM_RING_READ) {
            /* Success read, add read pos */
            eal_fetch_and_add64(&list->read_pos, i+1);
            *task = cur_task;
            ret = 0;
            break;
        }
    }

    return ret;
}

/* Release the task to queue */
forceinline int lmice_release_task(lmshd_task_t* task)
{
    eal_compare_and_swap64(&task->lock, LM_RING_READING, LM_RING_NOT_USE);
    return 0;
}

/* Pick an empty task */
forceinline int lmice_pick_task(lmshd_ring_t* list, lmshd_task_t**task)
{
    size_t i = 0;
    int ret = 1;
    int64_t val;
    uint64_t cur_pos = list->write_pos;
    lmshd_task_t* cur_task = NULL;
    for(i = 0; i< TASKPROC_LIST_SIZE; ++i,++cur_pos) {
        cur_task = list->task + (cur_pos & (TASKPROC_LIST_SIZE-1) );
        val = eal_compare_and_swap64(&cur_task->lock, LM_RING_NOT_USE, LM_RING_WRITING);
        if(val == LM_RING_NOT_USE) {
            /* Success read, add read pos */
            eal_fetch_and_add64(&list->write_pos, i+1);
            *task = cur_task;
            ret = 0;
            break;
        }
    }

    return ret;
}

/* Post a task to ring */
forceinline int lmice_post_task(lmshd_task_t* task)
{
    eal_compare_and_swap64(&task->lock, LM_RING_WRITING, LM_RING_WRITE);
    return 0;
}

forceinline int lmice_check_condition(lmshd_task_t* task, lmflt_cond_t *cond) {
    int ret = 1;
    lmnet_pkg_t * pkg = (lmnet_pkg_t*)task->pdata;
    /* Check predefined cond */
    if(pkg) {
        if(cond->flt & LMICE_FILTER_DATA_BIG_ENDIAN && pkg->endian == eal_is_big_endian())
            ret = 0;
    }
    /* Check tid */
    if ( (cond->tid.data_id == 0 || cond->tid.data_id == task->tid.data_id)
         &&(cond->tid.type_id == 0 || cond->tid.type_id == task->tid.type_id)
         &&(cond->tid.tick_time == 0 || cond->tid.tick_time == task->tid.tick_time)
         &&(cond->tid.object_id == 0 || cond->tid.object_id == task->tid.object_id) ) {
        ret = 0;
    }

    return ret;
}

/* scheduled task processing routine */
void schedule_task_proc(void* pdata) {
    int i = 0;
    int ret = 0;
    int level_step = 1;
    lmshd_task_t * task = NULL;
    lm_res_param_t* pm = (lm_res_param_t*) pdata;
    lmshd_ring_t* list = &pm->taskproc_list;
    lmflt_list_t *flt_list = NULL;
    lmflt_t *filter = NULL;
    uint32_t flt_pos;
    int term_flag = 0;
    lmflt_tls_t *tls = NULL;

    tls = (lmflt_tls_t*)malloc(sizeof(lmflt_tls_t));
    tls->mesg_rbtree = NULL;
    tls->resset_rbtree = NULL;
    eal_set_tls_value(task_filter_key, tls);


    for(;;) {

        /* check quit flag */
        if(pm->taskproc_quit_flag == 1)
            break;

        /* wait for task queue */
        eal_event_wait_one(pm->taskproc_evt);

        /* acquire a task from queue */
        ret = lmice_acquire_task(list, &task);
        if(ret == 0) {  /* pick a new task */

            /* reset termination flag */
            term_flag = 0;

            /* process level */
            if(task->pd == LMICE_TASK_PROC_IN) {
                i = 0;
                level_step = 1;
            } else {
                i = 3;
                level_step = -1;
            }
            for(; i <= 3 && i >= 0 ; i+= level_step) {

                /* termination filter list */
                flt_list = LMICE_GET_FILTER_LIST(pm->taskfilter_list,
                                                 task->pd,
                                                 LMICE_TASK_PROC_TERMINATE,
                                                 i);
                do {
                    for(flt_pos = 0; flt_pos < flt_list->size; ++flt_pos) {
                        filter = &flt_list->flt_list[flt_pos];
                        /* check condition */
                        ret = lmice_check_condition(task, &filter->cond);
                        /* call filter callback */
                        if(ret == 0) {
                            ret = filter->cb(task, filter->pdata);
                        }
                        /* check return value */
                        if(ret != 0) {
                            term_flag = 1;
                            break; /* break-for: filter */
                        }
                    }
                    flt_list = flt_list->next;
                } while(flt_list != NULL);

                if(term_flag == 1)
                    break; /* break-for: level */

                /* pass filter list */
                flt_list = &pm->taskfilter_list[task->pd +
                        (LMICE_TASK_PROC_PASS<<1) +
                        (i<<2)];
                do {
                    for(flt_pos = 0; flt_pos < flt_list->size; ++flt_pos) {
                        filter = &flt_list->flt_list[flt_pos];
                        /* check condition */
                        ret = lmice_check_condition(task, &filter->cond);
                        /* call filter callback */
                        if(ret == 0) {
                            ret = filter->cb(task, filter->pdata);
                        }
                    }
                    flt_list = flt_list->next;
                } while(flt_list != NULL);

            }/* end-for: level */

            /* release the task */
            lmice_release_task(task);

        } /* end-if acquire task */
    } /* end-for: endless */

    free(tls);
}

/* 创建任务处理线程池 */
forceinline int create_task_threads(lm_res_param_t* pm) {
    int ret = 0;
    uint32_t lcore = 0;
    uint32_t mem = 0;
    uint32_t bandwidth = 0;
    uint32_t threads = 0;
    size_t i = 0;
    lm_thread_ctx_t* ctx;

    /* Step1 get logical_cores */
    eal_core_get_properties(&lcore, &mem, &bandwidth);
    /* Step2 create threads */
    /* thread nums = lcore*2+2 */
    threads = lcore*2+2;

    pm->taskproc_size = DEFAULT_LIST_SIZE>threads?threads:DEFAULT_LIST_SIZE;
    for(i=0; i< pm->taskproc_size; ++i) {
        eal_thread_malloc_context(ctx);
        ctx->context = pm;
        ctx->handler = schedule_task_proc;
        ret = eal_thread_create(&pm->taskproc_thread[i], ctx);
        if(ret != 0) {/* create thread failed */
            lmice_critical_print("create_task_threads call eal_thread_create(%lu) failed[%d]\n", i, ret);
        }
    }
    return ret;
}

forceinline void stop_task_threads(lm_res_param_t* pm) {
    uint64_t i = 0;
    pm->taskproc_quit_flag = 1;
    for(i=0; i< pm->taskproc_size*2; ++i) {
        eal_event_awake(pm->taskproc_evt);
    }

    for(i = 0; i < pm->taskproc_size; ++ i) {
        eal_thread_join( pm->taskproc_thread[i], NULL );
    }
}

int create_schedule_service(lm_res_param_t* pm) {
    int ret = 0;
    lm_time_param_t *m_time = &pm->tm_param;
    lm_server_t *m_server = NULL;
    lm_shm_res_t *m_resource = &pm->res_server;


    /* Create tasklist event */
    {
        uint64_t eid;
        lmice_event_t evt;
        eid = eal_hash64_fnv1a(LMICE_TASKLIST_EVENT_NAME, sizeof(LMICE_TASKLIST_EVENT_NAME)-1);
        eal_event_zero(&evt);
        eal_event_hash_name(eid, evt.name);

        ret = eal_event_create(&evt);
        if(ret != 0) {
            lmice_debug_print("create tasklist event(%s) failed[%d]\n",evt.name, ret);
            eal_event_destroy(&evt);
            return ret;
        }
        pm->taskproc_evt = evt.fd;
    }

    /* Step1 create task threads */
    ret = create_task_threads(pm);

    /* Step2 create io-event thread */
    ret = create_io_thread(pm);

    /* Apply system timer resource */
    memset(m_time, 0, sizeof(lm_time_param_t));
    m_server = (lm_server_t*)((void*)(m_resource->addr));
    m_time->pt = &m_server->tm;

    /* Step3 create time and timer thread */
    ret = create_time_thread(pm);
    if(ret != 0){
        lmice_critical_print("Create time service failed[%d]\n", ret);
    }
    return ret;
}

int destroy_schedule_service(lm_res_param_t* pm)
{

    int ret = 0;
    uint64_t eid;
    lmice_event_t evt;
    eid = eal_hash64_fnv1a(LMICE_TASKLIST_EVENT_NAME, sizeof(LMICE_TASKLIST_EVENT_NAME)-1);
    eal_event_zero(&evt);
    eal_event_hash_name(eid, evt.name);

    lmice_debug_print("stop_time_thread\n");
    ret = stop_time_thread(pm);

    lmice_debug_print("stop_io_thread\n");
    ret |= stop_io_thread(pm);

    lmice_debug_print("stop_task_threads\n");
    stop_task_threads(pm);

    /* Close tasklist event */
    ret |= eal_event_close(pm->taskproc_evt);
    eal_event_destroy(&evt);
    return ret;
}
