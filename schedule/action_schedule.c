
#include "action_schedule.h"
#include "timer/timer_system_time.h"

#include "eal/lmice_trace.h"
#include "eal/lmice_eal_common.h"

static int forceinline create_iocp_handle(HANDLE* cp)
{
    int ret = 0;
    DWORD err = 0;

    *cp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0);
    /* 检查创建IO内核对象失败*/
    if (NULL == *cp) {
        err = GetLastError();
        lmice_error_print("CreateIoCompletionPort failed. Error[%u]\n", err);
        ret = -1;
    }
    return ret;
}

int create_schedule_service(lm_res_param_t* pm) {
    int ret = 0;
    lm_time_param_t *m_time = &pm->tm_param;
    lm_server_t *m_server = NULL;
    lm_shm_res_t *m_resource = &pm->res_server;

    /* 创建iocp */
    ret = create_iocp_handle(&pm->cp);

    /* 启动时间维护服务 */
    memset(m_time, 0, sizeof(lm_time_param_t));
    m_server = (lm_server_t*)((void*)(m_resource->addr));
    m_time->pt = &m_server->tm;
    ret = create_time_thread(pm);
    if(ret != 0){
        lmice_critical_print("Create time service failed[%d]", ret);
    }
    return ret;
}

int destroy_schedule_service(lm_res_param_t* pm)
{
    int ret = 0;
//    lm_time_param_t *m_time = &pm->tm_param;
    ret = stop_time_thread(pm);
    return ret;
}
