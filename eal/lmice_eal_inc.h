#ifndef LMICE_EAL_INC_H
#define LMICE_EAL_INC_H

/** Inter-node communication */
#include "lmice_eal_common.h"

struct eal_inc_param_s
{
    uint64_t id;
    /* processed by getaddrinfo-bind routine
     * [in] local socket port
     * [in] local socket address
     * Empty - any available port
     *       - any available address
     */
    char local_port[8];
    char local_addr[64];
    /* processed by
     * TCP-connect routine,
     * UDP-send routine
     * MC-add-membership routine
     * [in] remote socket port
     * [in] remote socket address
     * Empty - any available port
     *       - any available address
     */
    char remote_port[8];
    char remote_addr[64];

    int ttl;
    int padding0;

    int sock_client;
    int sock_server;

    struct addrinfo *remote;
};
typedef struct eal_inc_param_s eal_inc_param;


#if defined(_WIN32)
#include <lmice_eal_wsa.h>
#define eal_inc_init eal_wsa_init
#define eal_inc_finit eal_wsa_finit

#elif defined(__APPLE__)

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define eal_inc_init()
#define eal_finc_init()

#endif

int eal_inc_create_client(eal_inc_param* pm);
int eal_inc_create_server(eal_inc_param *pm);

#endif /* LMICE_EAL_INC_H */

