#ifndef RESOURCE_RIGHT_H
#define RESOURCE_RIGHT_H

/** 资源权限 */

#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_spinlock.h"
#include "eal/lmice_eal_hash.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define LMICE_RESOURCE_RIGHT_PUBLISH 1
#define LMICE_RESOURCE_RIGHT_SUBSCRIBE 2

#define LMICE_RESOURCE_RIGHT_SUBLIST_SIZE 32
#define LMICE_RESOURCE_RIGHT_OBJLIST_SIZE 32

#define LMRES_ERR_SUCCESS 0
#define LMRES_ERR_ALREADY 1
#define LMRES_ERR_NOTFOUND 2

/** 类型: 信息类型
 * 用以区分不同结构的信息
 * 如xx信息、定时器信息
*/

/** 对象: 系统实体
 * 用以区分不同的实体
*/

/** 权限: 资源的访问能力
 * 权限:为工作实例分配
 * 发布、订阅两类
*/

/** 资源: 某一对象的某一类型信息
 * 包含:      发布、订阅权限列表
 *          类型
 *          对象
 *
*/
struct lmice_resource_s {
    uint64_t id;            /* resource identity    */
    uint64_t type_id;       /* type identity        */
    uint64_t object_id;     /* object identity      */

    /* right set (1 publish, many subscribe */
    uint64_t pub_worker;
    uint64_t sub_size;
    uint64_t *sub_worker;
};
typedef struct lmice_resource_s lm_res_t;

#define LMICE_RESOURCE_RIGHT_LIST_SIZE 84
struct lmice_resource_list_s {
    volatile int64_t lock;
    struct lmice_resource_list_s *next;
    lm_res_t res_right[LMICE_RESOURCE_RIGHT_LIST_SIZE];
};
typedef struct lmice_resource_list_s lm_res_list_t;

/** 资源集合：同一类型的信息集合
 * 包含: 订阅权限列表
 *      类型
*/
struct lmice_resource_set_s {
    volatile int64_t lock;
    uint64_t id;            /* resource identity    */
    uint64_t type_id;       /* type identity        */

    /* right set(many subscribes) */
    uint64_t sub_size;
    uint64_t *sub_worker;
    /* object set(many objects) */
    uint64_t obj_size;
    uint64_t *object_id;
};
typedef struct lmice_resource_set_s lm_resset_t;

#define LMICE_RESOURCESET_RIGHT_LIST_SIZE 72
struct lmice_resourceset_list_s {
    volatile int64_t lock;
    struct lmice_resourceset_list_s *next;
    lm_resset_t res_right[LMICE_RESOURCESET_RIGHT_LIST_SIZE];
};
typedef struct lmice_resourceset_list_s lm_resset_list_t;

forceinline void lmres_create_id(uint64_t type_id, uint64_t object_id, uint64_t* res_id)
{
    *res_id = eal_hash64_fnv1a(&type_id, sizeof(type_id));
    *res_id = eal_hash64_more_fnv1a(&object_id, sizeof(object_id), *res_id);
}

forceinline void lmresset_create_id(uint64_t type_id, uint64_t* res_id)
{
    *res_id = eal_hash64_fnv1a(&type_id, sizeof(type_id));
}

forceinline int lmres_acquire_publish_right(lm_res_list_t* rr_list, uint64_t type_id, uint64_t object_id, uint64_t worker_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i = 0;
    lm_res_t *res = rr_list->res_right;
    lm_res_t *empty = NULL;
    volatile int64_t* lock = &rr_list->lock;

    eal_spin_lock(lock);

    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCE_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->object_id == object_id && res->type_id == type_id) {
                /* check right */
                if(res->pub_worker == 0) {
                    res->pub_worker = worker_id;
                    ret = LMRES_ERR_SUCCESS;
                    break;
                } else {
                    /* already been assigned */
                    ret = LMRES_ERR_ALREADY;
                    break;
                }
            } else if(res->id == 0 && empty == NULL) {
                /* the first empty res_right */
                empty = res;
            }
        }
        if(ret != LMRES_ERR_NOTFOUND) {
            break;
        }
        if( rr_list->next != 0) {
            /* finding next list */
            rr_list = rr_list->next;
        } else if(rr_list->next == 0 && empty != NULL) {
            /* create new resource right */
            lmres_create_id(type_id, object_id, &empty->id);
            empty->object_id = object_id;
            empty->type_id = type_id;
            empty->pub_worker = worker_id;
            ret = LMRES_ERR_SUCCESS;
            break;
        } else {
            /* create new list */
            rr_list->next = (lm_res_list_t*)malloc(sizeof(lm_res_list_t) );
            memset(rr_list->next, 0, sizeof(lm_res_list_t) );
            rr_list = rr_list->next;
        }

    }while(rr_list != NULL);

    eal_spin_unlock(lock);

    return ret;
}
forceinline int lmres_release_publish_right(lm_res_list_t* rr_list, uint64_t type_id, uint64_t object_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i = 0;
    lm_res_t *res = NULL;
    volatile int64_t* lock = &rr_list->lock;

    eal_spin_lock(lock);

    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCE_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->object_id == object_id && res->type_id == type_id) {
                /* check right */
                res->pub_worker = 0;
                ret = LMRES_ERR_SUCCESS;
                break;
            }
        }
        if(ret == LMRES_ERR_SUCCESS) {
            break;
        }

        rr_list = rr_list->next;

    }while(rr_list != NULL);

    eal_spin_unlock(lock);

    return ret;
}

forceinline int lmres_add_subscribe_right_list(uint64_t* size, uint64_t** pplist, uint64_t worker_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i=0;
    uint64_t *sub_worker = *pplist;
    uint64_t *empty = NULL;
    for(i=0; i< *size; ++i, ++sub_worker) {
        if(*sub_worker == worker_id) {
            ret = LMRES_ERR_SUCCESS;
            break;
        } else if(*sub_worker == 0 && empty == NULL) {
            empty = sub_worker;
        }
    }
    if(ret == LMRES_ERR_SUCCESS) {

    } else if(empty) {
        *empty = worker_id;
        ret = LMRES_ERR_SUCCESS;
    } else {
        *size += LMICE_RESOURCE_RIGHT_SUBLIST_SIZE;
        sub_worker = malloc((*size)*sizeof(uint64_t));
        memcpy(sub_worker, *pplist, ((*size) - LMICE_RESOURCE_RIGHT_SUBLIST_SIZE)*sizeof(uint64_t));
        free(*pplist);
        *pplist = sub_worker;
        *(*pplist + (*size)-LMICE_RESOURCE_RIGHT_SUBLIST_SIZE) = worker_id;
        ret = LMRES_ERR_SUCCESS;
    }

    return ret;

}

forceinline int lmres_del_subscribe_right_list(uint64_t size, uint64_t* sub_list, uint64_t worker_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i=0;
    uint64_t *sub_worker= sub_list;
    for(i=0; i<size; ++i, ++sub_worker) {
        if(*sub_worker == worker_id) {
            *sub_worker = 0;
            ret = LMRES_ERR_SUCCESS;
            break;
        }
    }
    return ret;
}

forceinline int lmres_acquire_subscribe_right(lm_res_list_t* rr_list, uint64_t type_id, uint64_t object_id, uint64_t worker_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i = 0;
    lm_res_t *res = NULL;
    lm_res_t *empty = NULL;
    volatile int64_t* lock = &rr_list->lock;

    eal_spin_lock(lock);

    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCE_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->object_id == object_id && res->type_id == type_id) {
                /* add subscribe right */
                lmres_add_subscribe_right_list(&res->sub_size, &res->sub_worker, worker_id);

                ret = LMRES_ERR_SUCCESS;
                break;

            } else if(res->id == 0 && empty == NULL) {
                /* the first empty res_right */
                empty = res;
            }
        }
        if(ret != LMRES_ERR_NOTFOUND) {
            break;
        }
        if( rr_list->next != 0) {
            /* finding next list */
            rr_list = rr_list->next;
        } else if(rr_list->next == 0 && empty != NULL) {
            /* create new resource right */
            lmres_create_id(type_id, object_id, &empty->id);
            empty->object_id = object_id;
            empty->type_id = type_id;
            lmres_add_subscribe_right_list(&empty->sub_size, &empty->sub_worker, worker_id);
            ret = LMRES_ERR_SUCCESS;
            break;
        } else {
            /* create new list */
            rr_list->next = (lm_res_list_t*)malloc( sizeof(lm_res_list_t) );
            memset(rr_list->next, 0, sizeof(lm_res_list_t) );
            rr_list = rr_list->next;
        }

    }while(rr_list != NULL);

    eal_spin_unlock(lock);

    return ret;
}

forceinline int lmres_release_subscribe_right(lm_res_list_t* rr_list, uint64_t type_id, uint64_t object_id, uint64_t worker_id) {

    int ret = LMRES_ERR_NOTFOUND;
    size_t i = 0;
    lm_res_t *res = NULL;
    volatile int64_t* lock = &rr_list->lock;

    eal_spin_lock(lock);

    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCE_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->object_id == object_id && res->type_id == type_id) {
                /* remove subscribe right */
                lmres_del_subscribe_right_list(res->sub_size, res->sub_worker, worker_id);
                ret = LMRES_ERR_SUCCESS;
                break;
            }
        }
        if(ret == LMRES_ERR_SUCCESS) {
            break;
        }

        rr_list = rr_list->next;

    }while(rr_list != NULL);

    eal_spin_unlock(lock);

    return ret;
}

forceinline int lmres_check_subscribe_right(lm_res_list_t* rr_list, uint64_t type_id, uint64_t object_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i = 0;
    lm_res_t *res = NULL;
    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCE_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->object_id == object_id && res->type_id == type_id) {
                /* got subscribe right */
                ret = LMRES_ERR_SUCCESS;
                break;
            }
        }
        if(ret == LMRES_ERR_SUCCESS) {
            break;
        }

        rr_list = rr_list->next;

    }while(rr_list != NULL);

    return ret;
}

forceinline int lmresset_acquire_subscribe_right(lm_resset_list_t* rr_list, uint64_t type_id, uint64_t worker_id) {
    volatile int64_t* lock = &rr_list->lock;
    lm_resset_t *res = NULL;
    lm_resset_t *empty = NULL;
    size_t i=0;
    int ret = LMRES_ERR_NOTFOUND;

    eal_spin_lock(lock);
    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCESET_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->type_id == type_id) {
                /* add subscribe right */
                eal_spin_lock(&res->lock);
                lmres_add_subscribe_right_list(&res->sub_size, &res->sub_worker, worker_id);
                eal_spin_unlock(&res->lock);

                ret = LMRES_ERR_SUCCESS;
                break;

            } else if(res->id == 0 && empty == NULL) {
                /* the first empty res_right */
                empty = res;
            }
        }
        if(ret != LMRES_ERR_NOTFOUND) {
            break;
        }
        if( rr_list->next != 0) {
            /* finding next list */
            rr_list = rr_list->next;
        } else if(rr_list->next == 0 && empty != NULL) {
            /* create new resource right */
            lmresset_create_id(type_id, &empty->id);
            empty->type_id = type_id;
            lmres_add_subscribe_right_list(&empty->sub_size, &empty->sub_worker, worker_id);
            ret = LMRES_ERR_SUCCESS;
            break;
        } else {
            /* create new list */
            rr_list->next = (lm_resset_list_t*)malloc( sizeof(lm_resset_list_t) );
            memset(rr_list->next, 0, sizeof(lm_resset_list_t) );
            rr_list = rr_list->next;
        }

    }while(rr_list != NULL);
    eal_spin_unlock(lock);

    return ret;
}

forceinline int lmresset_release_subscribe_right(lm_resset_list_t* rr_list, uint64_t type_id, uint64_t worker_id) {
    volatile int64_t* lock = &rr_list->lock;
    lm_resset_t *res = NULL;
    size_t i=0;
    int ret = LMRES_ERR_NOTFOUND;

    eal_spin_lock(lock);

    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCESET_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->type_id == type_id) {
                /* remove subscribe right */
                lmres_del_subscribe_right_list(res->sub_size, res->sub_worker, worker_id);
                ret = LMRES_ERR_SUCCESS;
                break;
            }
        }
        if(ret == LMRES_ERR_SUCCESS) {
            break;
        }

        rr_list = rr_list->next;

    }while(rr_list != NULL);

    eal_spin_unlock(lock);

    return ret;
}

forceinline int lmresset_check_subscribe_right(lm_resset_list_t* rr_list, uint64_t type_id) {
    lm_resset_t *res = NULL;
    size_t i=0;
    int ret = LMRES_ERR_NOTFOUND;
    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCESET_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->type_id == type_id) {
                /* got subscribe right */
                ret = LMRES_ERR_SUCCESS;
                break;
            }
        }
        if(ret == LMRES_ERR_SUCCESS) {
            break;
        }

        rr_list = rr_list->next;

    }while(rr_list != NULL);

    return ret;
}

forceinline int lmresset_add_object(uint64_t* size, uint64_t** pp_list, uint64_t object_id) {
    int ret = LMRES_ERR_NOTFOUND;
    size_t i = 0;
    uint64_t* obj_id = *pp_list;
    uint64_t* empty = NULL;
    for(i=0; i< *size; ++i, ++obj_id) {
        if(*obj_id == object_id) {
            /* already exist! */
            ret = LMRES_ERR_ALREADY;
            break;
        } else if(*obj_id ==0 && empty == NULL) {
            empty = obj_id;
        }
    }
    if(ret == LMRES_ERR_ALREADY) {

    } else if(empty) {
        *empty = object_id;
        ret = LMRES_ERR_SUCCESS;
    } else {
        /* create new */
        *size += LMICE_RESOURCE_RIGHT_OBJLIST_SIZE;
        obj_id = malloc((*size)*sizeof(uint64_t));
        memcpy(obj_id, *pp_list, ((*size) - LMICE_RESOURCE_RIGHT_SUBLIST_SIZE)*sizeof(uint64_t));
        free(*pp_list);
        *pp_list = obj_id;
        *(*pp_list + (*size)-LMICE_RESOURCE_RIGHT_SUBLIST_SIZE) = object_id;
        ret = LMRES_ERR_SUCCESS;
    }
    return ret;

}

forceinline int lmresset_add_subscribe_object(lm_resset_list_t* rr_list, uint64_t type_id, uint64_t object_id) {
    lm_resset_t *res = NULL;
    size_t i=0;
    int ret = LMRES_ERR_NOTFOUND;
    do {
        res = rr_list->res_right;
        for(i = 0; i < LMICE_RESOURCESET_RIGHT_LIST_SIZE; ++i, ++res) {
            if(res->type_id == type_id) {
                /* add subscribe right */
                eal_spin_lock(&res->lock);
                ret = lmresset_add_object(&res->obj_size, &res->object_id, object_id);
                eal_spin_unlock(&res->lock);

                break;
            }
        }
        if(ret != LMRES_ERR_NOTFOUND) {
            break;
        }

        rr_list = rr_list->next;

    }while(rr_list != NULL);

    return ret;
}

#endif /** RESOURCE_RIGHT_H */

