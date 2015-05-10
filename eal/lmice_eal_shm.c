#include "lmice_eal_common.h"
#include "lmice_eal_shm.h"
#include "lmice_eal_hash.h"
#include "lmice_trace.h"

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#if defined(__APPLE__) || defined(__LINUX__)

static forceinline
int eal_shm_open_with_mode(lmice_shm_t* shm, int mode) {
    int ret = 0;
    shm->fd = shm_open(shm->name, mode, 0600);
    if(shm->fd == -1) {
        lmice_debug_print("eal_shm_create call shm_open(%s) return fd(%d) and size(%d) errno(%d)", shm->name, shm->fd, shm->size, errno);
        shm->fd = 0;
        return errno;
    } else {
        ret = ftruncate(shm->fd, shm->size);
        LMICE_TRACE_COLOR_PRINT(lmice_trace_debug, "ftruncate fd(%d) and size(%d) errno(%d)", shm->fd, shm->size, errno);
        if(ret == 0) {
            shm->addr = mmap(NULL, shm->size, PROT_READ|PROT_WRITE,MAP_SHARED, shm->fd, 0);
            if((void*)shm->addr == MAP_FAILED) {
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
        eal_shm_close(shm->fd, shm->addr);
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
            shm->addr = mmap(NULL, shm->size, prot, MAP_SHARED, shm->fd, 0);
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

int eal_shm_close(shmfd_t fd, addr_t addr)
{
    int ret = 0;
    if(addr != 0) {
        ret = munmap(addr, 0);
        if(ret == -1) {
            lmice_debug_print("eal_shm_close call munmap error %d\n", errno);
        }
        /* reset shared memory address to zero */
        addr = 0;
    }

    if(fd != 0) {
        ret = close(fd);
        if(ret == -1) {
            lmice_debug_print("eal_shm_close call close error %d\n", errno);
        }
        fd = 0;
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

static forceinline
int eal_shm_open_with_mode(lmice_shm_t* shm, int mode)
{

    DWORD err=0;
    DWORD access = FILE_MAP_READ;

    if(mode & O_RDWR)
        access |= FILE_MAP_WRITE;

    shm->fd = OpenFileMappingA(
                access,
                FALSE,
                shm->name
                );
    if(shm->fd == NULL)
    {
        err = GetLastError();
        lmice_debug_print("eal_shm_create call OpenFileMapping(%s) return fd(%px) and size(%d) err(%ld)", shm->name, shm->fd, shm->size, err);

        return err;
    }

    shm->addr = MapViewOfFile(
                shm->fd, // handle to map object
                access,  // read/write permission
                0,
                0,
                shm->size);

    if (shm->addr == 0)
    {
        err = GetLastError();
        lmice_error_print("Could not map view of file (%lu).\n", err);

        eal_shm_close(shm->fd, shm->addr);

        return err;
    }

    return 0;
}

int eal_shm_create(lmice_shm_t* shm)
{
    HANDLE hMapFile;
    LPVOID pBuf;

    shm->fd = (HANDLE)0;

    hMapFile = CreateFileMappingA (
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                shm->size,               // maximum object size (low-order DWORD)
                shm->name);              // name of mapping object

    if (hMapFile == NULL)
    {
        DWORD err = GetLastError();
        lmice_error_print("Could not create file mapping object (%lu).\n", err);
        return 1;
    }

    pBuf = MapViewOfFile(
                hMapFile,               // handle to map object
                FILE_MAP_ALL_ACCESS,    // read/write permission
                0,
                0,
                shm->size);

    if (pBuf == NULL)
    {
        lmice_error_print("Could not map view of file (%lu).\n",GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    shm->fd = hMapFile;
    shm->addr = pBuf;

    return 0;

}

int eal_shm_destroy(lmice_shm_t* shm)
{
    if(shm->addr)
    {
        UnmapViewOfFile(shm->addr);
        shm->addr = 0;
    }

    if(shm->fd)
    {
        CloseHandle(shm->fd);
        shm->fd = NULL;
    }
    return 0;
}

int eal_shm_open(lmice_shm_t* shm, int mode)
{

    /** if shared memory address is already existing, then just return zero */
    if(shm->addr != 0 || shm->fd != 0)
        return 0;

    return eal_shm_open_with_mode(shm, mode);

}

int eal_shm_close(shmfd_t fd, addr_t addr)
{
    lmice_shm_t shm;
    shm.addr = addr;
    shm.fd = fd;
    return eal_shm_destroy(&shm);
}

void eal_shm_zero(lmice_shm_t* shm)
{
    memset(shm, 0, sizeof(lmice_shm_t));
}

int eal_shm_open_readonly(lmice_shm_t* shm)
{
    return eal_shm_open(shm, O_RDONLY);
}

int eal_shm_open_readwrite(lmice_shm_t* shm)
{
    return eal_shm_open(shm, O_RDWR);
}

#endif

static forceinline
void hash_to_nameA(uint64_t hval, char* name)
{
    int i=0;
    const char* hex_list="0123456789ABCDEF";
    for(i=0; i<8; ++i)
    {
        name[i*2] = hex_list[ *( (uint8_t*)&hval+i) >> 4];
        name[i*2+1] = hex_list[ *( (uint8_t*)&hval+i) & 0xf ];
    }
}

int eal_shm_hash_name(uint64_t hval, char *name)
{
#if defined(_WIN32)
    memcpy(name, "Global\\sm", 9);
    hash_to_nameA(hval, name+9);
    name[25]='\0';
#else
    memcpy(name, "sm", 2);
    hash_to_nameA(hval, name+2);
    name[18]='\0';
#endif


    return 0;
}
