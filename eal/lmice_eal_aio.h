#ifndef LMICE_EAL_AIO_H
#define LMICE_EAL_AIO_H

/** Async IO */


#if defined(_WIN32)
#include "lmice_eal_iocp.h"
#define eal_aio_data_list eal_iocp_data_list
#define eal_aio_handle eal_iocp_handle

#define eal_aio_create_handle eal_create_iocp_handle
#elif defined(__APPLE__)

#include "lmice_eal_kqueue.h"
#define eal_aio_data_list eal_kqueue_data_list
#define eal_aio_handle eal_kqueue_handle
#elif defined(__LINUX__)
#endif

int eal_aio_create_handle(eal_aio_handle* handle);

#endif /* LMICE_EAL_AIO_H */

