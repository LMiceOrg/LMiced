#ifndef NET_GROUP_ADDRESS_H
#define NET_GROUP_ADDRESS_H

#include <stdint.h>
#include "net_manage.h"

/* 分组地址表：用户维护 */
struct lmice_net_group_address_s {
    uint64_t session_id;    /* 0:internal 1..n:user select */
    lmnet_addr address;
};
typedef struct lmice_net_group_address_s lmnet_gaddr_t;

#define LMNET_GADDR_LENGTH 30

struct lmice_net_group_address_list_s {
    volatile int64_t lock;
    uint32_t size;          /* total size */
    uint32_t current;      /* current list empty gaddr's numbers */
    lmnet_gaddr_t array[LMNET_GADDR_LENGTH];
    struct lmice_net_group_address_list_s* next;

};
typedef struct lmice_net_group_address_list_s lmnet_galist_t;

int lmnet_append_net_group_address_config(lmnet_galist_t* galist, uint64_t sid, uint16_t ttl,
uint16_t port,
uint32_t proto,
uint8_t (address)[16]);

#endif /** NET_GROUP_ADDRESS_H */

