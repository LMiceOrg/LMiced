#ifndef NET_MANAGE_H
#define NET_MANAGE_H

#include <stdint.h>

#ifdef _WIN32

/**
  Windows IOCP
*/
//#include "net_manage_win.h"
#include "rtspace.h"

#endif

/** beatheart service */
int create_beatheart_service(uint64_t inst_id);
int destroy_beatheart_service(uint64_t inst_id);

/** ntp service */
int create_ntp_service(uint64_t inst_id);
int destroy_ntp_service(uint64_t inst_id);

/** pubmsg service
param1: socket id
param2: pubmsg_list */
int create_pubmsg_service(uint64_t inst_id);
int destroy_pubmsg_service(uint64_t inst_id);

int create_network_server(lm_mc_cfg_t *cfg);

#endif /** NET_MANAGE_H */

