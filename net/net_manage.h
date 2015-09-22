#ifndef NET_MANAGE_H
#define NET_MANAGE_H

#include <stdint.h>

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
 * 1.4 subnet: network identity
 * 1.5 message-length: 32 + data content's length
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
    uint8_t fragsize;   /* 分帧 总帧数  0:表示此数据报没有分帧 */
    uint8_t fragment;   /* 分帧 当前帧  */
    uint16_t version;   /* lmiced's version */
    uint16_t msglen;    /* 信息大小 */
};
typedef struct lmice_net_package_header_s lmnet_pkg_t;

/* 实体(obj)在T时刻(evt)发送消息(sys_type) */
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



#include "rtspace.h"


/** beatheart service */
/** ntp service */
/** pubmsg service */

struct lmice_net_address_s {
    uint16_t ttl;
    uint16_t port;
    uint16_t af;    /*address family: AF_INET AF_INET6 ... */
    uint16_t proto; /* protocols: IPPROTO_IP, IPPROTO_TCP, IPPROTO_UDP */
    uint8_t address[16];
};
typedef struct lmice_net_address_s lmnet_addr;

/**
插件机制：网络地址查询插件 类似DNS，
1：初始化：注册这7张表，注册发布事件回调，
2：当工作实例发布信息时，平台生成发布关系（默认地址），并产生发布事件
2.1：执行此回调，并获得网络地址，修改默认地址
3：最终，平台实现信息向这些网络地址的传送
管道过滤器机制
流出事件 --> 进入管道 --> 执行插件 -->流出管道 --> 执行传送


*/

/* 节点字典表：工作实例运行于特定节点,由心跳服务维护 */
struct lmice_net_node_s {
    uint64_t node_id;
    lmnet_addr address;

};

/* 主题字典表:由特定消息类型,消息实体的组合构成消息主题,由用户维护(运行前,运行中) */
struct lmice_net_topic_s {
    uint64_t topic_id;
    lmnet_addr address;
};

/* 工作实例字典表 */
struct lmice_net_work_s {
    uint64_t work_id;   /* 工作实例   */
    uint64_t work_type; /* 工作实例类型 */
};

/* 消息字典表：消息实例附属于特定消息类型 */
struct lmice_net_message_s {
    uint64_t message_type;
    uint64_t instance_id;
};

/* 实例信息表: 消息类型和实例包含于特定工作实例，由心跳服务维护*/
struct lmice_net_workinfo_s {
    struct lmice_net_node_s node;
    struct lmice_net_work_s work;
    struct lmice_net_message_s mesg;

};

/* 主题信息表：由特定消息类型,消息实体的组合构成消息主题,由用户维护(运行前,运行中) */
struct lmice_net_topicinfoinfo_s {
    struct lmice_net_topic_s topic;
    struct lmice_net_message_s mesg;
};



/* 地址映射表：类型对应分组地址 由用户维护，且平台自动分配 */
#include "net_group_address_map.h"

/* 分组地址列表（每会话独立） */
#include "net_group_address.h"

#endif /** NET_MANAGE_H */

