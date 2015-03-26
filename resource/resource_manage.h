#ifndef RESOURCE_MANAGE_H
#define RESOURCE_MANAGE_H

/** 资源包括 发布订阅的(共享内存)信息, 注册的定时器(周期性,一次性),注册的事件
 *
 *  信息分为元数据和信息内容两部分
 *
 * 运行时包含多个情景,每个情景相互独立
 *
 * 情景列表  默认情景(0) | scenlist.log
**/
#include "eal/lmice_eal_shm.h"
#include "eal/lmice_eal_event.h"

#include "timer/timer_system_time.h"

#include <stdint.h>

#define CLIENT_SHMNAME  "CC597303-0F85-40B2-8BDC-4724BD" /** C87B4E */
#define BOARD_SHMNAME   "82E0EE49-382C-40E7-AEA2-495999" /** 392D29 */
#define DEFAULT_SHM_SIZE 4096 /** 4KB */

struct lmice_instance_info_s
{
    /* always be 1, the info struct version */
    uint32_t version;
    uint32_t reserved;

    /*  client process id if equal to zero(0) means rtspace maintain the resource */
    uint64_t process_id;
    /*  client thread id, if equal to zero(0) means any thread in the process */
    uint64_t thread_id;
    /* client type, user defined */
    uint64_t type_id;
    /*client or server instance identity */
    uint64_t instance_id;

};
typedef struct lmice_instance_info_s lm_instance_info_t;

struct lmice_server_info_s
{
    /* always be 1, the info struct version */
    uint32_t version;
    /* server shm block size*/
    uint32_t size;
    /* next server info identity, zero(0) means no extra server info block */
    uint64_t next_info_id;

    /* lock */
    uint64_t lock;
    /* event identity */
    uint64_t event_id;
    /* system/tick time resource */
    lm_time_t tm;
    /* variant number of instances info */
    lm_instance_info_t inst;
};
typedef struct lmice_server_info_s lm_server_info_t;

struct lmice_resourece_s
{
    lmice_event_t   evt;
    lmice_shm_t     shm;
};
typedef struct lmice_resourece_s lm_resourece_t;

int create_server_resource(lm_resourece_t* server);
int destroy_server_resource(lm_resourece_t* server);

#endif /** RESOURCE_MANAGE_H */

