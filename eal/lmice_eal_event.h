#ifndef LMICE_EAL_EVENT_H
#define LMICE_EAL_EVENT_H

#include <stdint.h>
struct lmice_event_s
{
    int fd;
    char name[32];
};

int eal_event_create();
int eal_event_awake(uint64_t eid);

#endif /** LMICE_EAL_EVENT_H */

