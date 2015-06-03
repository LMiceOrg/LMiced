#include "io_schedule.h"

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
