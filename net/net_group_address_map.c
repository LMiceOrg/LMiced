#include "net_group_address_map.h"

int lmnet_append_net_group_address_map(lmnet_gamap_t* gamap, uint64_t sid, uint16_t ttl,
uint16_t port,
uint32_t proto,
uint8_t (address)[16]) {
    lmnet_gamap_t *cur = gamap;

    eal_spin_lock(&gamap->lock);
    for(;;) {
        if(cur->current < LMNET_GADDR_MAP_LENGTH){
            cur->array[cur->current].id = sid;
            cur->array[cur->current].address.ttl = ttl;
            cur->array[cur->current].address.port = port;
            cur->array[cur->current].address.proto = proto;
            memset(cur->array[cur->current].address.address, address, 16);

            ++cur->current;
            ++gamap->size;
            break;
        } else if(cur->next) {
            cur = cur->next;
        } else {
            cur->next = (lmnet_gamap_t*)malloc(sizeof(lmnet_gamap_t));
            memset(cur->next, 0, sizeof(lmnet_gamap_t));
            cur = cur->next;
        }
    }

    eal_spin_unlock(&gamap->lock);
    return 0;

}
