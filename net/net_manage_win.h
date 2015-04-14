#ifndef NET_MANAGE_WIN_H
#define NET_MANAGE_WIN_H

#include "eal/lmice_eal_common.h"
#include "eal/lmice_ring.h"
#include "eal/lmice_trace.h"
#include "eal/lmice_eal_spinlock.h"



/**
 * Socket编程需用的动态链接库
pragma comment(lib, "Ws2_32.lib")
    IOCP需要用到的动态链接库
pragma comment(lib, "Kernel32.lib")
*/

/**
 * lmice_io_data_s
 * 重叠I/O需要用到的结构体，临时记录IO数据
 **/
const int LMICE_MAX_NET_PACKAGE_SIZE  = 64 * 1024;
struct lmice_io_data_s
{
    OVERLAPPED overlapped;
    WSABUF databuff;
    char buffer[ LMICE_MAX_NET_PACKAGE_SIZE ];
    int BufferLen;
    int operationType;
};
typedef struct lmice_io_data_s lm_io_data_t;

/**
 * lmice_handle_data_s
 * 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
 * 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
 **/
struct eal_wsa_handle_s
{
    int64_t state;
    SOCKET nfd;
    SOCKADDR_STORAGE client_addr;
};
typedef struct eal_wsa_handle_s eal_wsa_handle;

enum lmice_handle_data_e
{
    LMICE_HANDLE_DATA_NOTUSE = 0,
    LMICE_HANDLE_DATA_USING = 1,
    LMICE_CLIENT_LIST_SIZE = 32
};

struct lmice_handle_data_head_s
{
    volatile int64_t    lock;
    eal_wsa_handle*   next;
};

typedef struct lmice_handle_data_head_s lm_hd_head_t;


struct lmice_net_parameter_s
{
    int worker_count;
    HANDLE complete_port;
    eal_wsa_handle *client_list;
};

typedef struct lmice_net_parameter_s lm_net_param_t;


void forceinline create_handle_data_list(eal_wsa_handle** cl)
{
    *cl = (eal_wsa_handle*)malloc( sizeof(eal_wsa_handle)*LMICE_CLIENT_LIST_SIZE );
    memset(*cl, 0, sizeof(eal_wsa_handle)*LMICE_CLIENT_LIST_SIZE );
}

void forceinline delete_handle_data_list(eal_wsa_handle* cl)
{
    lm_hd_head_t* head = NULL;
    eal_wsa_handle* next = NULL;
    do {
        head = (lm_hd_head_t*)cl;
        next = head->next;
        free(cl);
        cl = next;
    } while(cl != NULL);
}

int forceinline create_handle_data(eal_wsa_handle* cl, eal_wsa_handle** val)
{


    lm_hd_head_t* head = NULL;
    eal_wsa_handle* cur = NULL;
    size_t i = 0;

    do {
        head = (lm_hd_head_t*)cl;
        for( i= 1; i < LMICE_CLIENT_LIST_SIZE; ++i) {
            *cur = cl+i;
            if(cur->state == LMICE_HANDLE_DATA_NOTUSE) {
                cur->state = LMICE_HANDLE_DATA_USING;
                *val = cur;
                return 0;
            }
        }
        cl = head->next;
    } while(cl != NULL);

    create_handle_data_list(&cur);
    head->next = cur;
    *val = cur+1;
    return 0;
}

int forceinline delete_handle_data(eal_wsa_handle* cl, const eal_wsa_handle* val)
{
    lm_hd_head_t* head = NULL;
    eal_wsa_handle* cur = NULL;
    size_t i = 0;
    head = (lm_hd_head_t*)cl;
    do {
        for( i= 1; i < LMICE_CLIENT_LIST_SIZE; ++i) {
            *cur = cl+i;
            if(cur->state == LMICE_HANDLE_DATA_USING &&
                    cur == val) {
                cur->state = LMICE_HANDLE_DATA_NOTUSE;
                return 0;
            }
        }
        cl = head->next;
    } while(cl != NULL);

    return 1;
}

// 定义全局变量
vector < eal_wsa_handle* > clientGroup;		// 记录客户端的向量组



HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);



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

static int forceinline create_worker_thread(int count, HANDLE cp)
{
    int ret = 0;
    int i=0;
    DWORD err = 0;

    for(i = 0; i < count; ++i) {
        HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, cp, 0, NULL);
        if(NULL == ThreadHandle) {
            err = GetLastError();
            lmice_error_print("Create Thread Handle failed. Error[%u]\n", err);
            ret = -1;
        }
        CloseHandle(ThreadHandle);
    }

    return ret;
}

enum lmice_net_tcp
{
    LMICE_TCP_CLIENT_MODE,
    LMICE_TCP_SERVER_MODE
};

struct eal_wsa_service_param_s
{
    /* 0 client-mode, 1 server-mode */
    int mode;
    /* ipv4, ipv6 */
    int inet;
    char bind_port[8];
    char bind_addr[64];

    SOCKET    nfd;
    HANDLE  cp;

};
typedef struct eal_wsa_service_param_s eal_wsa_service_param;

void forceinline zero_net_tcp_param(eal_wsa_service_param* pm)
{
    memset(pm, 0, sizeof(eal_wsa_service_param));
}

DWORD WINAPI tcp_accept_thread_proc( LPVOID lpParameter)
{
    eal_wsa_service_param * pm = (eal_wsa_service_param *)lpParameter;
    int ret = 0;

    eal_wsa_handle* PerHandleData;

    for(;;) {
        // 接收连接，并分配完成端，这儿可以用AcceptEx()
        SOCKET acceptSocket;
        SOCKADDR_STORAGE saRemote;
        int RemoteLen;
        RemoteLen = sizeof(saRemote);
        acceptSocket = accept(pm->nfd, (struct sockaddr*)&saRemote, &RemoteLen);
        if(SOCKET_ERROR == acceptSocket){	// 接收客户端失败
            ret = WSAGetLastError();
            lmice_error_print("Accept Socket Error[%d] ", ret);
            return -1;
        }

        // 创建用来和套接字关联的单句柄数据信息结构
        // 在堆中为这个PerHandleData申请指定大小的内存
        PerHandleData = (eal_wsa_handle*)malloc(sizeof(eal_wsa_handle));
        memset(PerHandleData, 0, sizeof(eal_wsa_handle));
        PerHandleData -> nfd = acceptSocket;
        memcpy (&PerHandleData -> client_addr, &saRemote, RemoteLen);
        // 将单个客户端数据指针放到客户端组中


        // 将接受套接字和完成端口关联
        CreateIoCompletionPort((HANDLE)(PerHandleData->nfd),
                               pm->cp,
                               PerHandleData,
                               0);


        // 开始在接受套接字上处理I/O使用重叠I/O机制
        // 在新建的套接字上投递一个或多个异步
        // WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务
        // 单I/O操作数据(I/O重叠)
        lm_io_data_t* PerIoData = NULL;
        PerIoData = (lm_io_data_t*)malloc(sizeof(lm_io_data_t));
        memset(PerIoData, 0, sizeof(lm_io_data_t));
        PerIoData->databuff.len = LMICE_MAX_NET_PACKAGE_SIZE;
        PerIoData->databuff.buf = PerIoData->buffer;
        PerIoData->operationType = 0;	// read

        DWORD RecvBytes;
        DWORD Flags = 0;
        WSARecv(PerHandleData->nfd, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);

    }
    return 0;
}

int forceinline create_tcp_accept_thread(eal_wsa_service_param * pm)
{
    int ret = 0;
    HANDLE thd;
    thd = CreateThread(NULL,
                       0,
                       tcp_accept_thread_proc,
                       pm,
                       0,
                       NULL);
    if(thd == NULL)
    {
        ret = 1;
    }
    CloseHandle(thd);
    return ret;
}

int forceinline create_tcp_service(eal_wsa_service_param * pm)
{
    int ret = 0;
    struct addrinfo *result = NULL;
    struct addrinfo *rp = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* TCP protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    ret = getaddrinfo(pm->bind_addr, pm->bind_port, &hints, &result);
    if (ret != 0) {
        lmice_error_print("getaddrinfo[%s]: %d\n", pm->bind_addr, ret);
        return -1;
    }

    /* getaddrinfo() returns a list of address structures.
                 Try each address until we successfully bind(2).
                 If socket(2) (or bind(2)) fails, we (close the socket
                 and) try the next address.
    */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        ret = -1;
        pm->inet = rp->ai_family;
        pm->nfd = WSASocket(rp->ai_family,
                            rp->ai_socktype,
                            rp->ai_protocol,
                            NULL,
                            0,
                            WSA_FLAG_OVERLAPPED);

        if (INVALID_SOCKET == pm->nfd)
            continue;

        if(pm->mode == LMICE_TCP_CLIENT_MODE) {
            /* 连接 server */
            ret = WSAConnect(pm->nfd,
                             rp->ai_addr,
                             rp->ai_addrlen,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
            if(ret == SOCKET_ERROR)
            {
                ret = WSAGetLastError();
                lmice_error_print("Connect failed. Error[%u]", ret);
                closesocket(pm->nfd);
                continue;
            }
        } else if(pm->mode == LMICE_TCP_SERVER_MODE) {

            /* 绑定SOCKET到地址 */
            ret = bind(pm->nfd, rp->ai_addr, rp->ai_addrlen);
            if (ret == SOCKET_ERROR)
            {
                ret = WSAGetLastError();
                lmice_error_print("Bind failed. Error[%u]", ret);
                closesocket(pm->nfd);
                continue;
            }

            /* 将SOCKET设置为监听模式 */
            ret = listen(pm->nfd, 10);
            if(ret == SOCKET_ERROR) {
                ret = WSAGetLastError();
                lmice_error_print("Listen failed. Error[%u]", ret);
                closesocket(pm->nfd);
                continue;
            }

            ret = create_tcp_accept_thread(pm);


        }
        ret = 0;
        break;

    } /* end-for: rp */

    /* No longer needed */
    freeaddrinfo(result);

    return ret;
}

int forceinline init_net_iocp()
{
    WORD wVersionRequested = MAKEWORD(2, 2); // 请求2.2版本的WinSock库
    WSADATA wsaData;	// 接收Windows Socket的结构信息
    int ret;

    /** 加载socket动态链接库*/
    ret = WSAStartup(wVersionRequested, &wsaData);

    /* 检查套接字库是否申请成功 */
    if (0 != ret){
        lmice_error_print("WSAStartup Windows Socket Library Error[%u]!\n", ret);
        return -1;
    }

    /* 检查是否申请了所需版本的套接字库 */
    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){
        WSACleanup();
        lmice_error_print("Request Windows Socket Version 2.2 Error!\n");
        return -1;
    }

    return 0;
}

void forceinline finit_net_iocp()
{
    WSACleanup();
}

// 开始主函数
int create_net_iocp()
{
    int ret = 0;
    HANDLE cp;

    /** 初始化 socket */
    ret = init_net_iocp();


    /** 创建IOCP的内核对象 */
    ret = create_iocp_handle(&cp);

    /** 创建IOCP线程--线程里面创建线程池 */
    ret = create_worker_thread(4, cp);

    //    // 确定处理器的核心数量
    //    SYSTEM_INFO mySysInfo;
    //    GetSystemInfo(&mySysInfo);

    //    // 基于处理器的核心数量创建线程
    //    for(DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i){
    //        // 创建服务器工作器线程，并将完成端口传递到该线程
    //        HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, NULL);
    //        if(NULL == ThreadHandle){
    //            err = GetLastError();
    //            lmice_error_print("Create Thread Handle failed. Error[%u]\n", err);
    //            return -1;
    //        }
    //        CloseHandle(ThreadHandle);
    //    }

    // 建立流式套接字
    eal_wsa_service_param pm;
    zero_net_tcp_param(&pm);
    memcpy(pm.bind_addr, "0.0.0.0", 7);
    memcpy(pm.bind_port, "30001", 5);
    pm.mode = LMICE_TCP_SERVER_MODE;
    ret = create_tcp_service(&pm);

    // 开始处理IO数据
    cout << "本服务器已准备就绪，正在等待客户端的接入...\n";

    while(true){
        eal_wsa_handle * PerHandleData = NULL;
        SOCKADDR_IN saRemote;
        int RemoteLen;
        SOCKET acceptSocket;

        // 接收连接，并分配完成端，这儿可以用AcceptEx()
        RemoteLen = sizeof(saRemote);
        acceptSocket = accept(srvSocket, (SOCKADDR*)&saRemote, &RemoteLen);
        if(SOCKET_ERROR == acceptSocket){	// 接收客户端失败
            cerr << "Accept Socket Error: " << GetLastError() << endl;
            system("pause");
            return -1;
        }

        // 创建用来和套接字关联的单句柄数据信息结构
        // 在堆中为这个PerHandleData申请指定大小的内存
        PerHandleData = (eal_wsa_handle*)malloc(sizeof(eal_wsa_handle));
        memset(PerHandleData, 0, sizeof(eal_wsa_handle));
        PerHandleData -> nfd = acceptSocket;
        memcpy (&PerHandleData -> client_addr, &saRemote, RemoteLen);
        clientGroup.push_back(PerHandleData);		// 将单个客户端数据指针放到客户端组中

        // 将接受套接字和完成端口关联
        CreateIoCompletionPort((HANDLE)(PerHandleData -> nfd), completionPort, (DWORD)PerHandleData, 0);


        // 开始在接受套接字上处理I/O使用重叠I/O机制
        // 在新建的套接字上投递一个或多个异步
        // WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务
        // 单I/O操作数据(I/O重叠)
        lm_io_data_t* PerIoData = NULL;
        PerIoData = (lm_io_data_t*)malloc(sizeof(lm_io_data_t));
        memset(PerIoData, 0, sizeof(lm_io_data_t));
        PerIoData->databuff.len = 1024;
        PerIoData->databuff.buf = PerIoData->buffer;
        PerIoData->operationType = 0;	// read

        DWORD RecvBytes;
        DWORD Flags = 0;
        WSARecv(PerHandleData->nfd, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
    }

    system("pause");
    return 0;
}

// 开始服务工作线程函数
DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
    HANDLE CompletionPort = (HANDLE)IpParam;
    DWORD BytesTransferred;
    LPOVERLAPPED IpOverlapped;
    eal_wsa_handle* PerHandleData = NULL;
    lm_io_data_t* PerIoData = NULL;
    DWORD RecvBytes;
    DWORD Flags = 0;
    BOOL bRet = false;

    while(true){
        bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
        if(bRet == 0){
            cerr << "GetQueuedCompletionStatus Error: " << GetLastError() << endl;
            return -1;
        }
        PerIoData = (lm_io_data_t*)CONTAINING_RECORD(IpOverlapped, lm_io_data_t, overlapped);

        // 检查在套接字上是否有错误发生
        if(0 == BytesTransferred){
            closesocket(PerHandleData->nfd);
            GlobalFree(PerHandleData);
            GlobalFree(PerIoData);
            continue;
        }

        // 开始数据处理，接收来自客户端的数据
        WaitForSingleObject(hMutex,INFINITE);
        cout << "A Client says: " << PerIoData->databuff.buf << endl;
        ReleaseMutex(hMutex);

        // 为下一个重叠调用建立单I/O操作数据
        ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
        PerIoData->databuff.len = 1024;
        PerIoData->databuff.buf = PerIoData->buffer;
        PerIoData->operationType = 0;	// read
        WSARecv(PerHandleData->nfd, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
    }

    return 0;
}


// 发送信息的线程执行函数
DWORD WINAPI ServerSendThread(LPVOID IpParam)
{
    while(1){
        char talk[200];
        gets(talk);
        int len;
        for (len = 0; talk[len] != '\0'; ++len){
            // 找出这个字符组的长度
        }
        talk[len] = '\n';
        talk[++len] = '\0';
        printf("I Say:");
        cout << talk;
        WaitForSingleObject(hMutex,INFINITE);
        for(int i = 0; i < clientGroup.size(); ++i){
            send(clientGroup[i]->socket, talk, 200, 0);	// 发送信息
        }
        ReleaseMutex(hMutex);
    }
    return 0;
}

#endif /** NET_MANAGE_WIN_H */

