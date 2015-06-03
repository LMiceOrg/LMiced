#ifndef FILTER_TLS_H
#define FILTER_TLS_H

#include "resource/resource_manage.h"
#include "sglib.h"

#include <stdint.h>

/* rb-tree <object, type> -> mesg_res */
struct lmice_mesg_res_rbtree_s {
    uint64_t object_id;
    uint64_t type_id;
    lm_mesg_res_t *res;
    int color_field;
    int padding0;
    struct lmice_mesg_res_rbtree_s* left;
    struct lmice_mesg_res_rbtree_s* right;
};
typedef struct lmice_mesg_res_rbtree_s lm_mesg_rbtree;

#define lmice_obj_type_rbtree_cmparator(x,y) ( ((int64_t)x->object_id)-((int64_t)y->object_id) ) + \
    ( ((int64_t)x->type_id) - ((int64_t)y->type_id) )

SGLIB_DEFINE_RBTREE_PROTOTYPES(lm_mesg_rbtree, left, right, color_field, lmice_obj_type_rbtree_cmparator)

/* rb-tree <object, type> ->resset */
struct lmice_resset_rbtree_s {
    uint64_t object_id;
    uint64_t type_id;
    int color_field;
    int padding0;
    struct lmice_resset_rbtree_s* left;
    struct lmice_resset_rbtree_s* right;
};
typedef struct lmice_resset_rbtree_s lm_resset_rbtree;

SGLIB_DEFINE_RBTREE_PROTOTYPES(lm_resset_rbtree, left, right, color_field, lmice_obj_type_rbtree_cmparator)

struct lmice_filter_tls_s {
    lm_mesg_rbtree * mesg_rbtree;
    lm_resset_rbtree* resset_rbtree;
};
typedef struct lmice_filter_tls_s lmflt_tls_t;

#endif /** FILTER_TLS_H */
