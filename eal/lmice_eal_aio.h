#ifndef LMICE_EAL_AIO_H
#define LMICE_EAL_AIO_H

/** Async IO */

#if defined(_WIN32)
#include "lmice_eal_iocp.h"
#define eal_aio_data_list eal_iocp_data_list
#define eal_aio_handle HANDLE

#define eal_aio_create_handle eal_create_iocp_handle
#else
#endif

#endif /* LMICE_EAL_AIO_H */

