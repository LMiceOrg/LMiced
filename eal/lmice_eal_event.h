#ifndef LMICE_EAL_EVENT_H
#define LMICE_EAL_EVENT_H

#include "lmice_eal_common.h"
#include <stdint.h>
#ifdef __LINUX__
struct lmice_event_s
{
    int fd;
    char name[32];
};
#elif defined(_WIN32)

#ifdef _W64
struct lmice_event_s
{
    HANDLE fd;
    char name[32];
};
#else

struct lmice_event_s
{
    HANDLE fd;
    int32_t padding;
    char name[32];
};
#endif

#endif

typedef struct lmice_event_s lmice_event_t;

int eal_event_zero(lmice_event_t* e);

int eal_event_create(lmice_event_t *e);
int eal_event_destroy(uint64_t eid);
int eal_event_awake(uint64_t eid);

int eal_event_hash_name(uint64_t hval, char *name);

#endif /** LMICE_EAL_EVENT_H */

