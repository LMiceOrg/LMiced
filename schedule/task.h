#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define data_id_t uint64_t
#define type_id_t uint64_t
#define tick_id_t uint64_t
#define object_id_t uint64_t

/* Task identity */
struct lmice_task_identity_s
{
    data_id_t data_id;
    type_id_t type_id;
    tick_id_t tick_time;
    object_id_t object_id;
};
typedef struct lmice_task_identity_s lmtsk_id_t;

struct lmice_task_s
{
    lmtsk_id_t id;
    uint64_t size;
    void* data;
};
typedef struct lmice_task_s lmtsk_t;

#endif /** TASK_H */
