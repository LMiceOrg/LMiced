#include "task_filter.h"
#include "resource/resource_manage.h"
#include "resource/message_resource.h"
#include "system/system_syscall.h"
#include "eal/lmice_eal_thread.h"
#include "filter_tls.h"

/**
 * @brief lmflt_subscribe_type type level[1] dir[in/out] mode[pass] filter
 * update object id list
 * @param ptask resource'type task
 * @param pdata res_param
 * @return 0:success
 */
int lmflt_subscribe_type(void* ptask, void* pdata)
{
    int ret = 0;
    lm_res_param_t* pm = (lm_res_param_t*)pdata;
    lmtsk_t* task = (lmtsk_t*)ptask;
    lmflt_tls_t *tls = NULL;
    lm_resset_rbtree e;
    lm_resset_rbtree *ite = NULL;

    /* check task size */
    if(task->size == 0)
        return 0;

    eal_get_tls_value(task_filter_key, lmflt_tls_t*, tls);
    /* find in TLS resset rbtree */
    e.type_id = task->id.type_id;
    e.object_id = task->id.object_id;
    ite = sglib_lm_resset_rbtree_find_member(tls->resset_rbtree, &e);
    if(ite == NULL) { /* do add it in server resource */
        ret = lmresset_add_subscribe_object(&pm->resset_right, task->id.type_id, task->id.object_id);

        if(ret == 0) { /* update TLS rbtree */
            ite = (lm_resset_rbtree*)malloc(sizeof(lm_resset_rbtree));
            ite->object_id = task->id.object_id;
            ite->type_id = task->id.type_id;
            sglib_lm_resset_rbtree_add(&tls->resset_rbtree, ite);
        }
    }
    return 0;
}

/**
 * @brief lmflt_subscribe_object level[3] dir[in/out] mode[pass] filter
 * move data from wr-ring to rd-ring
 * @param ptask the data comes from publish task
 * @param pdata the task context, res_param
 * @return 0:success, else failed
 */
int lmflt_subscribe_object(void* ptask, void* pdata)
{
    int ret = 0;
    lm_res_param_t* pm = (lm_res_param_t*)pdata;
    lmtsk_t* task = (lmtsk_t*)ptask;
    lm_mesg_res_t *res = NULL;
    lmmsg_t * msg;
    lmmsg_blob_t *blob = NULL;
    lmflt_tls_t *tls = NULL;
    lm_mesg_rbtree e;
    lm_mesg_rbtree *ite = NULL;

    /* check task size */
    if(task->size == 0)
        return 0;

    eal_get_tls_value(task_filter_key, lmflt_tls_t*, tls);
    /* find object in TLS message tree */
    e.object_id = task->id.object_id;
    e.type_id = task->id.type_id;
    ite = sglib_lm_mesg_rbtree_find_member(tls->mesg_rbtree, &e);
    if( ite!= NULL) { /*got it */
        res = ite->res;
        if(res->info->inst_id == 0) { /* the res is deleted */
            sglib_lm_mesg_rbtree_delete(&tls->mesg_rbtree, &e);
            free(ite);
            return 0;
        }
        msg = (lmmsg_t*)res->addr;
    } else { /* not found */

        /* check the task had been subscribed */
        ret = lmresset_check_subscribe_right(&pm->resset_right, task->id.type_id);
        if(ret != 0) {
            ret = lmres_check_subscribe_right(&pm->res_right, task->id.type_id, task->id.object_id);
            if(ret != 0) {
                return 0;
            }
        }
        /* find subscribe message resource */
        ret = lmres_find_message_from_workers(pm->res_worker, task->id.type_id, task->id.object_id, res);
        if(ret != 0) { /* not found */
            /* create subscribe message resource */
            ret = lmsys_register_subscribe(&pm->res_right, pm->res_worker, task->id.type_id, task->id.object_id, 0, res);
            if(ret != 0) { /*create failed */
                return ret;
            }
            msg = (lmmsg_t*)res->addr;
            lmmsg_init_by_size(msg, res->info->size, task->size);
        } else {
            msg = (lmmsg_t*)res->addr;
        }

        /* add it to TLS message rbtree */
        ite = (lm_mesg_rbtree*)malloc(sizeof(lm_mesg_rbtree));
        ite->object_id = task->id.object_id;
        ite->type_id = task->id.type_id;
        ite->res = res;
        sglib_lm_mesg_rbtree_add(&tls->mesg_rbtree, ite);
    } /* end-else: not-found ite */

    /* peek a block from rd-ring */
    ret = lmmsg_acquire_unique_blob(msg, blob);
    if(ret !=0)
        return ret;
    /* update data */
    memcpy(blob, task->data, task->size);
    /* release the block */
    lmmsg_release_unique_blob(blob);

    return 0;
}
