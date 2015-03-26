#include "rtspace.h"

#include "resource/resource_manage.h"
#include "timer/timer_system_time.h"
#include "trust/trust_manage.h"

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
            printf("RT-Space: Version 1.0\n");
        }
    }
    else
    {

        int ret = 0;
        lm_resourece_t m_resource;
        time_param_t m_time;
        lm_trust_t m_trust;
        lm_server_info_t *m_server;

        /* 资源管理 */
        memset(&m_resource, 0, sizeof(m_resource));
        ret = create_server_resource(&m_resource);
        if(ret != 0)
        {
            lmice_critical_print("Create resource service failed[%d]\n", ret);
            return 1;
        }


        /* 时间管理 */
        memset(&m_time, 0, sizeof(m_time));
        m_server = (lm_server_info_t*)((void*)(m_resource.shm.addr));
        m_time.pt = &m_server->tm;
        ret = create_time_thread(&m_time);
        if(ret != 0)
        {
            lmice_critical_print("Create time service failed[%d]", ret);
            return 1;
        }

        /* 任务调度服务 */


        /* 可信计算服务 */
        memset(&m_trust, 0, sizeof(m_trust));
        m_trust.server = m_server;
        ret = create_trust_thread(&m_trust);
        if(ret != 0)
        {
            lmice_critical_print("Create trust service failed[%d]", ret);
            return 1;
        }

        getchar();

        stop_trust_thread(&m_trust);
        stop_time_thread(&m_time);
        destroy_server_resource(&m_resource);

    }


    return 0;
}

