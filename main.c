#include "rtspace.h"

#include "resource/resource_manage.h"
#include "trust/trust_manage.h"
#include "schedule/action_schedule.h"
#include "net/net_manage.h"

#include "eal/lmice_eal_common.h"
#include "eal/lmice_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/** network server */
int create_network_server(lm_res_param_t *pm);
int stop_network_server(lm_res_param_t *pm);

eal_tls_t task_filter_key;

int main(int argc, char* argv[]) {

    pthread_key_create(&task_filter_key, NULL);

    if(argc > 1) {
        if(strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("LMiced: Version 1.0\n");
        }
    } else {
        int ret = 0;
        lm_server_t *m_server = NULL;
        lm_res_param_t* res_param = NULL;
        lm_trust_t m_trust;


        /* 资源管理服务 */
        res_param = (lm_res_param_t*)malloc(sizeof(lm_res_param_t));
        memset(res_param, 0, sizeof(lm_res_param_t));
        ret = create_resource_service(res_param);
        if(ret != 0) {
            lmice_critical_print("Create resource service failed[%d]\n", ret);
            return 1;
        }

        m_server = (lm_server_t*)((void*)(res_param->res_server.addr));
        lmice_debug_print("schedule create\n");
        /* 任务调度服务 */
        ret = create_schedule_service(res_param);
        if(ret != 0) {
            lmice_critical_print("Create schedule service failed[%d]\n", ret);
            return 1;
        }


        lmice_debug_print("trust create\n");
        /* 可信计算服务 */
        memset(&m_trust, 0, sizeof(m_trust));
        m_trust.server = m_server;
        m_trust.efd = res_param->res_server.efd;
        ret = create_trust_thread(&m_trust);
        if(ret != 0) {
            lmice_critical_print("Create trust server failed[%d]\n", ret);
            return 1;
        }

        /* 节点间网络通讯服务 */
        {
        eal_inc_param *bh = &res_param->bh_param;
        memcpy(bh->local_addr, "127.0.0.1", 9);
        memcpy(bh->local_port, "0", 1);
        memcpy(bh->remote_addr, "230.0.0.1", 9);
        memcpy(bh->remote_port, "30001", 5);
        }
        //ret = create_network_server(res_param);
        if(ret != 0) {
            lmice_critical_print("Create inter-node communication service failed[%d]\n", ret);
            return 1;
        }

        getchar();

        //stop_network_server(res_param);
        stop_trust_thread(&m_trust);
        destroy_schedule_service(res_param);
        destroy_resource_service(res_param);


        free(res_param);

    }

    pthread_key_delete(task_filter_key);
    lmice_critical_print("LMiced shutdown service\n");
    return 0;
}

