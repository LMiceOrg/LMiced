#ifndef LMICE_EAL_INC_H
#define LMICE_EAL_INC_H

/** Inter-node communication */
#include <lmice_eal_common.h>

struct eal_inc_param_s
{
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
};
typedef struct eal_inc_param_s eal_inc_param;


#if defined(_WIN32)
#include <lmice_eal_wsa.h>
#define eal_inc_init eal_wsa_init
#define eal_inc_finit eal_wsa_finit

#endif



#endif /* LMICE_EAL_INC_H */

