#ifndef LMICE_EAL_WSA_H
#define LMICE_EAL_WSA_H

/* Windows Socket Library API */

#include "eal/lmice_trace.h"
#include "eal/lmice_eal_common.h"

int forceinline lmice_init_wsa()
{
    /* 请求2.2版本的WinSock库 */
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    int ret;

    /* 加载socket动态链接库 */
    ret = WSAStartup(version, &data);

    /* 检查套接字库是否申请成功 */
    if (0 != ret){
        lmice_error_print("WSAStartup Windows Socket Library Error[%u]!\n", ret);
        return -1;
    }

    /* 检查是否申请了所需版本的套接字库 */
    if(LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2){
        WSACleanup();
        lmice_error_print("Request Windows Socket Version 2.2 Error!\n");
        return -1;
    }

    return 0;
}

void forceinline lmice_finit_wsa()
{
    WSACleanup();
}

#endif /** LMICE_EAL_WSA_H */

