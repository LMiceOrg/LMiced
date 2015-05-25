#ifndef LMICE_EAL_KQUEUE_H
#define LMICE_EAL_KQUEUE_H

#include "lmice_eal_common.h"
#include <stdint.h>
struct eal_kqueue_data_list_s {
    uint64_t id;
};

#define eal_kqueue_handle int


#endif /** LMICE_EAL_KQUEUE_H */
