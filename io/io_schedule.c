#include "io_schedule.h"

#if defined(__APPLE__)
void io_thread_proc(void* pdata)
{
    lm_res_param_t* pm = (lm_res_param_t*)pdata;
    lm_io_param_t *io = &pm->io_param;
    struct kevent64_s chglist[8];
    int nchglist = 8;
    struct kevent64_s newevt[8];
    struct timespec ts;
    int newlist = 0;
    int i=0;
    int ret = 0;
    char buff[4096];

    ts.tv_nsec = 0;
    ts.tv_sec = 1;

    /* timed wait for new data arrival */
    ret = kevent64(io->cp, newevt, newlist, chglist, nchglist,  0, &ts);
    if(ret > 0) {
        for(i=0; i< ret; ++i) {
            lmice_critical_print("kevent[%d] return the data size %lld\n", i, chglist[i].data);
            recv(chglist[i].ident, buff, chglist[i].data, 0);
        }
    }



}

#elif defined(_WIN32)
void io_thread_proc(void* pdata) {
    HANDLE CompletionPort = (HANDLE)pdata;
    DWORD BytesTransferred;
    LPOVERLAPPED IpOverlapped;
    eal_wsa_handle* PerHandleData = NULL;
    /*lm_io_data_t* PerIoData = NULL;*/
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

        // 为下一个重叠调用建立单I/O操作数据
        ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
        PerIoData->databuff.len = 1024;
        PerIoData->databuff.buf = PerIoData->buffer;
        PerIoData->operationType = 0;	// read
        WSARecv(PerHandleData->nfd, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
    }

    return 0;
}

#endif
int create_io_thread(lm_res_param_t* pm)
{
    int ret = 0;

    lm_aio_ctx_t *ctx = NULL;
    lm_io_param_t *io = &pm->io_param;

    ret = eal_aio_create_handle(&io->cp);
    if(ret != 0) {
        lmice_error_print("create_io_thread call eal_aio_create_handle() failed[%d]\n", ret);
        return ret;
    }

    eal_aio_malloc_context(ctx);

    ctx->context = pm;
    ctx->handler = io_thread_proc;
    ctx->quit_flag = &(io->quit_flag);

    ret = eal_aio_create(&io->thd, ctx);

    return ret;
}

int stop_io_thread(lm_res_param_t *pm)
{
    lm_io_param_t *io = &pm->io_param;
    eal_aio_destroy(io);

    return 0;
}
