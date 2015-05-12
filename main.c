#include "rtspace.h"

#include "resource/resource_manage.h"
#include "trust/trust_manage.h"
#include "schedule/action_schedule.h"

#include "eal/lmice_eal_common.h"
#include "eal/lmice_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



int main(int argc, char* argv[])
{
    if(argc > 1)
    {
        if(strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
        {
            printf("LMiced: Version 1.0\n");
        }
    }
    else
    {
        int ret = 0;
        lm_server_t *m_server = NULL;
        lm_res_param_t* res_param = NULL;
        lm_trust_t m_trust;


        /* 资源管理服务 */
        res_param = (lm_res_param_t*)malloc(sizeof(lm_res_param_t));
        memset(res_param, 0, sizeof(lm_res_param_t));
        ret = create_resource_service(res_param);
        if(ret != 0)
        {
            lmice_critical_print("Create resource service failed[%d]\n", ret);
            return 1;
        }

        m_server = (lm_server_t*)((void*)(res_param->res_server.addr));
        lmice_debug_print("schedule create\n");
        /* 任务调度服务 */
        create_schedule_service(res_param);


        lmice_debug_print("trust create\n");
        /* 可信计算服务 */
        memset(&m_trust, 0, sizeof(m_trust));
        m_trust.server = m_server;
        m_trust.efd = res_param->res_server.efd;
        ret = create_trust_thread(&m_trust);
        if(ret != 0)
        {
            lmice_critical_print("Create trust service failed[%d]\n", ret);
            return 1;
        }

        lmice_debug_print("trust created\n");

        /* 节点间网络通讯服务 */

        getchar();

        stop_trust_thread(&m_trust);
        destroy_schedule_service(res_param);
        destroy_resource_service(res_param);
        free(res_param);

    }
    lmice_critical_print("LMiced shutdown service\n");
    return 0;
}

