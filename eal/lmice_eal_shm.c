#include "lmice_eal_common.h"
#include "lmice_eal_shm.h"
#include "lmice_trace.h"

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#if defined(__APPLE__) || defined(__linux__)

static inline forceinline
int eal_shm_open_with_mode(lmice_shm_t* shm, int mode)
{
    int ret = 0;
    shm->fd = shm_open(shm->name, mode, 0600);
    if(shm->fd == -1)
    {
        lmice_debug_print("eal_shm_create call shm_open(%s) return fd(%d) and size(%d) errno(%d)", shm->name, shm->fd, shm->size, errno);

        shm->fd = 0;
        return errno;
    }
    else
    {
        ret = ftruncate(shm->fd, shm->size);
        LMICE_TRACE_COLOR_PRINT(lmice_trace_debug, "ftruncate fd(%d) and size(%d) errno(%d)", shm->fd, shm->size, errno);
        if(ret == 0)
        {
            shm->addr = (uint64_t)mmap(NULL, shm->size, PROT_READ|PROT_WRITE,MAP_SHARED, shm->fd, 0);
            if((void*)shm->addr == MAP_FAILED)
            {
                ret = errno;
                shm->addr = 0;
            }

        }

    }
    return ret;
}

int eal_shm_create(lmice_shm_t* shm)
{
    return eal_shm_open_with_mode(shm, O_RDWR|O_CREAT);
}

int eal_shm_destroy(lmice_shm_t* shm)
{
    int ret;

    if(shm->addr != 0)
    {
        eal_shm_close(shm);
    }
    if(shm->fd != 0)
    {
        ret = close(shm->fd);
        if(ret == -1)
        {
            lmice_debug_print("close error %d", errno);
        }
        shm->fd = 0;
    }
    ret = shm_unlink(shm->name);
    return ret;
}

int eal_shm_open(lmice_shm_t* shm, int mode)
{
    int ret = 0;

    /** if shared memory address is already existing, then just return zero */
    if(shm->addr != 0)
        return 0;

    /** if fd is open, so do mmap directly */
    if(shm->fd != 0)
    {
        struct stat st;
        st.st_size = 0;
        ret = fstat(shm->fd, &st);
        if(ret == 0)
        {
            int prot = PROT_READ;
            if( (mode & O_RDWR) == O_RDWR)
                prot = PROT_READ|PROT_WRITE;

            shm->size = st.st_size;            
            shm->addr = (uint64_t)mmap(NULL, shm->size, prot, MAP_SHARED, shm->fd, 0);
            if((void*)shm->addr == MAP_FAILED)
            {
                lmice_error_print("eal_shm_open call mmap(%d) failed", shm->fd);
                shm->addr = 0;
                return errno;
            }
        }
        else
        {
            lmice_error_print("eal_shm_open call fstat(%d) failed", shm->fd);
            return errno;
        }
    }
    else
    {
        return eal_shm_open_with_mode(shm, mode);
    }
    return 0;
}

int eal_shm_close(lmice_shm_t* shm)
{
    int ret = 0;
    if(shm->addr != 0)
    {
        ret = munmap((void*)shm->addr, shm->size);
        if(ret == -1)
        {
            lmice_debug_print("eal_shm_close call munmap error %d\n", errno);
        }

        /** reset shared memory address to zero */
        shm->addr = 0;
    }

    if(shm->fd != 0)
    {
        ret = close(shm->fd);
        if(ret == -1)
        {
            lmice_debug_print("eal_shm_close call close error %d\n", errno);
        }
        shm->fd = 0;
    }
    return ret;
}


void eal_shm_zero(lmice_shm_t *shm)
{
    memset(shm, 0, sizeof(lmice_shm_t) );
}

int eal_shm_open_readonly(lmice_shm_t* shm)
{
    return eal_shm_open(shm, O_RDONLY);
}

int eal_shm_open_readwrite(lmice_shm_t* shm)
{
    return eal_shm_open(shm, O_RDWR);
}

#elif defined(_WIN32)

int eal_shm_create(lmice_shm_t* shm)
{

}

int eal_shm_destroy(lmice_shm_t* shm)
{

}

int eal_shm_open(lmice_shm_t* shm, int mode)
{

}

int eal_shm_close(lmice_shm_t* shm)
{

}

void eal_shm_zero(lmice_shm_t* shm)
{

}

int eal_shm_open_readonly(lmice_shm_t* shm)
{

}

int eal_shm_open_readwrite(lmice_shm_t* shm)
{

}

#endif
