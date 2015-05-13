#ifndef NET_BEATHEART_H
#define NET_BEATHEART_H

#include <eal/lmice_eal_common.h>
#include <eal/lmice_eal_hash.h>
#include <eal/lmice_eal_wsa.h>

#include "resource/resource_manage.h"

#include "net_manage.h"

/* beatheart type signature :
 * LMice name
 * LMice version
 * constant string
*/
#define LMNET_BEATHEART_TYPE "e4c9f551-20dc-4272-9675-7315c024eea4"
forceinline uint64_t lmice_net_beatheart_type(void)
{
    uint64_t hval = 0;
    uint32_t ver = LMICE_VERSION;
    hval = eal_hash64_fnv1a(LMICE_NAME, sizeof(LMICE_NAME) -1);
    hval = eal_hash64_more_fnv1a(&ver, 4, hval);
    hval = eal_hash64_more_fnv1a(LMNET_BEATHEART_TYPE, sizeof(LMNET_BEATHEART_TYPE) -1, hval);
    return hval;
}

/* beatheart instance signature :
 * net address
 * net configure
*/
forceinline uint64_t lmice_net_beatheart_inst(uint32_t net_cfg, const uint8_t* net_addr)
{
    uint64_t hval = 0;
    hval = eal_hash64_fnv1a(&net_cfg, 4);
    hval = eal_hash64_more_fnv1a(net_addr, 16, hval);
    return hval;
}

/* beatheart data content struct
 * NET info
 * Host info
 * Loan info
*/
struct lmice_net_beatheart_data_content_s
{
    uint32_t    size;
    uint32_t    net_cfg;
    uint8_t     net_addr[16];

    uint32_t    worker_size;
    uint32_t    lcore_size;
    uint32_t    memory_size;
    uint32_t    net_bankwidth;

    uint32_t    worker_usage;
    uint32_t    lcore_usage;
    uint32_t    memory_usage;
    uint32_t    net_usage;

};
typedef struct lmice_net_beatheart_data_content_s lmnet_bh_ctn_t;

/* net configure */
#define LMICE_NET_CFG_IPV4 0
#define LMICE_NET_CFG_IPV6 (1<<0)
#define LMICE_NET_CFG_AREA(x) (x) << 24  /* x in range[0, 255] */


struct lmice_net_beatheart_message_s
{
    /* lmnet_mh_t */
    uint64_t sys_type;
    uint64_t evt_tick;
    uint64_t obj_inst;
    /* lmnet_ct_t */
    lmnet_bh_ctn_t ctn;
};
typedef struct lmice_net_beatheart_message_s lmnet_bh_msg_t;

struct lmice_net_beatheart_package_s
{
    /* lmnet_ph_t */
    uint8_t  endian;
    uint8_t  padding;
    uint16_t headlen;
    uint32_t version;
    char meta_data[16]; /* i[b16]iiiiiii */
    lmnet_bh_msg_t  msg;
};
typedef struct lmice_net_beatheart_package_s lmnet_bh_pkg_t;

#define LMNET_BEATHEART_METADATA "i[b16]iiiiiii"

#define LMNET_BEATHEART_LIST_SIZE 36
struct lmice_net_beatheart_param_s
{
    lm_worker_t*    worker;
    eal_wsa_service_param net_param;
    lmnet_bh_pkg_t bh_packge;
    lmnet_bh_pkg_t bh_pkg_list[LMNET_BEATHEART_LIST_SIZE];
};

typedef struct lmice_net_beatheart_param_s lmnet_bh_prm_t;


/* initialize beatheart resource */
int lmnet_beatheart_init(lmnet_bh_prm_t* param);

/* finalize beatheart resource */
int lmnet_beatheart_final(lmnet_bh_prm_t* bh_param);

/** server mode beartheart routine
 * process incoming beatheart package
 * mantain inter-node state
 * dispatch received data to IO event-poll
*/

/* create server role */
int lmnet_beatheart_server_create(lmnet_bh_prm_t* param);

/* delete server role */
int lmnet_beatheart_server_delete(lmnet_bh_prm_t* param);


/** client mode beartheart routine
 * init beatheart package
 * init net info
 * init host info
 * gather host loan state
 * timely send [this] node beatheart package
*/

/* create beatheart client role */
int lmnet_beatheart_client_create(lmnet_bh_prm_t* bh_param);

/* delete beatheart client role */
int lmnet_beatheart_client_delete(lmnet_bh_prm_t* bh_param);

#endif /* NET_BEATHEART_H */

