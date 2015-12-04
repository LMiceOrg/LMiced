#ifndef NET_GROUP_ADDRESS_MAP_H
#define NET_GROUP_ADDRESS_MAP_H

#include <stdint.h>
#include "net_manage.h"

/* 地址映射表：类型对应分组地址 由用户维护，且平台自动分配 */
struct lmice_net_group_address_pair_s {
    uint64_t id;    /* topic_id , message_type */
    lmnet_addr address;
};
typedef struct lmice_net_group_address_pair_s lmnet_gapair_t;

#define LMNET_GADDR_MAP_LENGTH 30
struct lmice_net_group_address_map_s {
    volatile int64_t lock;
    uint32_t size;
    uint32_t current;
    lmnet_gapair_t array[30];
    struct lmice_net_group_address_map_s* next;
};
typedef struct lmice_net_group_address_map_s lmnet_gamap_t;

int lmnet_append_net_group_address_map(lmnet_gamap_t* gamap, uint64_t sid, uint16_t ttl,
uint16_t port,
uint32_t proto,
uint8_t (address)[16]);

#endif /** NET_GROUP_ADDRESS_MAP_H*/

