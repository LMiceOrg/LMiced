#include "net_group_address.h"

#include "eal/lmice_eal_spinlock.h"

#include <string.h>
#include <stdlib.h>
#define LMNET_NET_GROUP_CONFIG_KEY "net_group_config"
#define LMNET_NET_GROUP_CONFIG_KEY_LEN sizeof(LMNET_NET_GROUP_CONFIG_KEY) -1

int lmnet_append_net_group_address_config(lmnet_galist_t* galist, uint64_t sid, uint16_t ttl,
                                          uint16_t port,
                                          uint32_t proto,
                                          uint8_t (address)[16]) {
    lmnet_galist_t *cur = galist;

    eal_spin_lock(&galist->lock);
    for(;;) {
        if(cur->current < LMNET_GADDR_LENGTH){
            cur->array[cur->current].session_id = sid;
            cur->array[cur->current].address.ttl = ttl;
            cur->array[cur->current].address.port = port;
            cur->array[cur->current].address.proto = proto;
            memset(cur->array[cur->current].address.address, address, 16);

            ++cur->current;
            ++galist->size;
            break;
        } else if(cur->next) {
            cur = cur->next;
        } else {
            cur->next = (lmnet_galist_t*)malloc(sizeof(lmnet_galist_t));
            memset(cur->next, 0, sizeof(lmnet_galist_t));
            cur = cur->next;
        }
    }

    eal_spin_unlock(&galist->lock);
    return 0;
}
