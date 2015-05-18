#ifndef NET_MANAGE_H
#define NET_MANAGE_H

#include <stdint.h>
#include "resource/resource_manage.h"

/** NET service
 * description: inter-node communication
 * model: peer-to-peer without super node
 * service:
 * 1. beatheart
 * 2. synctime
 * 3. pub-sub
 *
 * net package struct:
 * 1. package-header
 * 1.1 endian: sender's endian
 * 1.2 padding: 1,2,4,8 ...
 * 1.3 version: LMICED_VERSION
 * 1.4 header-length: 8 + meta-data's length bytes( 8 bytes aligned)
 * 1.5 meta-data: message structure meta element
 * 2. message-header
 * 2.1 system type signature
 * 2.2 event tick signature
 * 2.3 object instance signature
 * 3. data-content
 * 3.1 data length
 * 3.2 content blob
 */

struct lmice_net_package_header_s
{
    uint8_t endian;
    uint8_t padding;
    uint16_t headlen;
    uint32_t version;
    char meta_data[8];
};
typedef struct lmice_net_package_header_s lmnet_pkg_t;

struct lmice_net_message_header_s
{
    uint64_t sys_type;
    uint64_t evt_tick;
    uint64_t obj_inst;
};
typedef struct lmice_net_message_header_s lmnet_msg_t;

struct lmice_net_data_content_s
{
    uint32_t size;
    char blob[4];
};
typedef struct lmice_net_data_content_s lmnet_ctn_t;


#ifdef _WIN32

/**
  Windows IOCP
*/
//#include "net_manage_win.h"
#include "rtspace.h"

#endif

/** beatheart service */
/** ntp service */
/** pubmsg service


/** network server */
int create_network_server(lm_res_param_t *pm);
int stop_network_server(lm_res_param_t *pm);
#endif /** NET_MANAGE_H */

