#ifndef LMICE_EAL_IOCP_H
#define LMICE_EAL_IOCP_H

#include "eal/lmice_trace.h"
#include "eal/lmice_eal_common.h"

/**
    IOCP需要用到的动态链接库
pragma comment(lib, "Kernel32.lib")
*/

/* iocp 操作指令 */
enum lmice_iocp_operation_e
{
    LMICE_TCP_RECV,
    LMICE_TCP_SEND,
    LMICE_MC_RECV,
    LMICE_MC_SEND,
    LMICE_UDP_RECV,
    LMICE_UDP_SEND
};

/* 临时记录IO数据 长度 */
#define LMICE_MAX_NET_PACKAGE_SIZE  (64 * 1024)

/* iocp 数据结构 */
struct lmice_iocp_data_s
{
    int operation;
    OVERLAPPED overlapped;
    WSABUF data;
    char buffer[ LMICE_MAX_NET_PACKAGE_SIZE ];

};
typedef struct lmice_iocp_data_s lm_iocp_dt;

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

#endif /** LMICE_EAL_IOCP_H */

