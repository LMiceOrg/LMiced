#ifndef RTSPACE_H
#define RTSPACE_H

#define LMICE_VERSION 1


enum lmice_error_code_e
{
    lm_err_network_failed
};


struct lmice_multicast_config
{
    /* UDP, TCP, Multicast */
    int ttl;
    /* Empty port - any available port */
    /* Empty addr - any available address */
    char local_port[8];
    char local_addr[64];
    char remote_port[8];
    char remote_addr[64];
};
typedef struct lmice_multicast_config lm_mc_cfg_t;

#endif /** RTSPACE_H */

