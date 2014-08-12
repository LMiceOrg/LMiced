#include "lmice_core.h"
#include "lmice_ring.h"
#include "lmice_trace.h"
#include "lmice_json_util.h"
#include "lmice_eal_endian.h"
#include "lmice_eal_shm.h"
#include "lmice_eal_hash.h"
#include "lmice_eal_align.h"
#include <stdio.h>

#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>


#include <pthread.h>

/**
 * @brief lmice_env the LMICE Abstract Environment Layer object.
 */
lmice_environment_t lmice_env;

static inline void __attribute__((always_inline))
lmice_env_init(lmice_environment_t* env)
{
    env->addr = NULL;
    env->pid = 0;
    env->ppid = 0;
    env->board = NULL;
    env->logfd = fopen("cmbstock.log", "a");
}

static inline void __attribute__((always_inline))
lmice_env_exit(lmice_environment_t* env)
{
    fclose(env->logfd);

    env->logfd = NULL;
    env->addr = NULL;
    env->pid = 0;
    env->ppid = 0;
    env->board = NULL;
}

static inline void __attribute__((always_inline))
lmice_config_init()
{
    json_t *json;
    json_t *obj;
    json_error_t error;
    const char *string_value;
    long long integer_value;

    json = json_load_file("lmice.config", JSON_DECODE_ANY, &error);
    if(!json)
        lmice_error_print("json_load_file returned non-NULL for a nonexistent file");
    if(error.line != -1)
        lmice_error_print("json_load_file returned an invalid line number [%d] with error[%s]", error.column, error.text);

    json_object_get_integer(json, obj, "log_file_size", integer_value);
    if(integer_value == 0)
    {
        lmice_critical_print("log_file_size section not found");
    }

    json_object_get_string(json, obj, "log_file", string_value);
    if(string_value == NULL)
    {
        lmice_critical_print("log_file section not found");
    }
    else
    {
        lmice_debug_print("log file is %s", string_value);
    }

    json_object_get_integer(json, obj, "log_file_size", integer_value);
    if(integer_value == 0)
    {
        lmice_critical_print("log_file_size section not found");
    }
    else
    {
        lmice_debug_print("log_file size is %lld", integer_value);
    }

    json_decref(json);

}


static inline int __attribute__((always_inline))
create_shm(lmice_shm_t* shm)
{
    int ret = 0;
    shm->fd = shm_open(shm->name, O_RDWR|O_CREAT, 0600);
    if(shm->fd == -1)
    {
        ret = -1;
    }
    else
    {
        ret = ftruncate(shm->fd, shm->size);
        if(ret == 0)
            shm->addr = (uintptr_t)mmap(NULL, shm->size, PROT_READ|PROT_WRITE,MAP_SHARED, shm->fd, 0);
    }
    return 0;
}

static inline int __attribute__((always_inline))
open_shm(lmice_shm_t* shm)
{
    int ret = 0;
    shm->fd = shm_open(shm->name, O_RDWR, 0600);
    if(shm->fd == -1)
    {
        ret = -1;
    }
    else
    {
        struct stat st;
        st.st_size = 0;
        ret = fstat(shm->fd, &st);
        if(ret == 0)
        {
            shm->size = st.st_size;
            printf("open size is %d\n", shm->size);
            shm->addr = (uintptr_t)mmap(NULL, shm->size, PROT_READ|PROT_WRITE,MAP_SHARED, shm->fd, 0);
        }
    }
    return ret;
}

static inline void* __attribute__((always_inline))
create_shm_file(const char* name, int size)
{
    void* addr = NULL;
    int fd = open(name, O_RDWR|O_CREAT, 0600);
    if(fd != -1)
    {
        ftruncate(fd, size);
        addr = mmap(NULL, size, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
    }
    return addr;
}

static inline off_t __attribute__((always_inline))
file_size(int fd)
{
    return lseek(fd, 0, SEEK_END);
}

static inline void* __attribute__((always_inline))
open_shm_file(const char* name)
{
    void* addr = NULL;
    int fd = open(name, O_RDWR, 0600);
    if(fd != -1)
    {
        off_t size = file_size(fd);
        addr = mmap(NULL, size, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
    }
    return addr;
}

static inline int __attribute__((always_inline))
close_shm(lmice_shm_t *shm)
{
    int ret = close(shm->fd);
    if(ret == -1)
    {
        printf("close error %d\n", errno);
    }
    ret |= munmap((void*)shm->addr, shm->size);
    if(ret == -1)
    {
        printf("munmap error %d\n", errno);
    }
    return ret;
}

static inline int __attribute__((always_inline))
destroy_shm(lmice_shm_t* shm)
{
    return shm_unlink(shm->name);
}

int main(int argc, char* const* argv)
{

    EAL_STRUCT_ALIGN(long);
#if 0
    //printf("endian %d\n", eal_is_big_endian());
//    int ret = 0;
//    lmice_shm_t shm;
//    eal_shm_zero(&shm);
//    sprintf(shm.name, "hellohellohellohellohell");
//    shm.size = 4096;
//    ret = eal_shm_create(&shm);
//    if(ret != 0)
//        return 0;
//    eal_shm_destroy(&shm);
    printf("%llu\n", eal_hash64_fnv1a("hello", 5));
    return 0;
//    /**
//     * @brief pthread_setname_np set the label of main thread.
//     */
//    pthread_setname_np("MainThread");

//    /**
//     *@brief lmice_env_init initialize the environment struct.
//     */
//    lmice_env_init(&lmice_env);
#endif
    /**
     *@brief process command line
     */
#if 0
//    if( lmice_process_cmdline(argc, argv) != 0)
//        return 1;

//    if( lmice_process_signal(&lmice_env) != 0)
//        return 1;

//    if( lmice_exec_command(&lmice_env) != 0)
//        return 1;

//    if( lmice_master_init(&lmice_env) != 0)
//        return 1;

//    if( lmice_signal_init(&lmice_env) != 0)
//        return 1;

//    /**
//     * @brief master thread loop
//     */
//    lmice_master_loop(&lmice_env);


/*
//    int ret = 0;
//    lmice_state_t *state, init_state;


//    struct rlimit rl;
//    ret = getrlimit(RLIMIT_NOFILE, &rl);
//    printf("%d limit %llu %llu\n", ret, rl.rlim_cur, rl.rlim_max);
//    rl.rlim_cur = 512;
//    ret = setrlimit(RLIMIT_NOFILE, &rl);
//    ret = getrlimit(RLIMIT_NOFILE, &rl);
//    printf("%d limit %llu %llu\n", ret, rl.rlim_cur, rl.rlim_max);

//    ret = getdtablesize();
//    printf("table size %d\n", ret);

//    ret = shm_unlink("aaa.bin");
//    printf("ret is %d\n", ret);

//    memset(&init_state, 0, sizeof(init_state));
//    state = &init_state;

//    lmice_init(state);

//    lmice_ring_t ring;
//    lmice_ring_create(&ring, 10,10);
//    lmice_ring_destroy(ring);

//    lmice_shm_t shm, shm2;
//    memset(&shm, 0, sizeof(lmice_shm_t));
//    memset(&shm2, 0, sizeof(lmice_shm_t));

//    shm.size = 4096;
//    sprintf(shm.name, "aaa.bin");
//    sprintf(shm2.name, "aaa.bin");

//    ret = create_shm(&shm);
//    printf("create shm %d\n", ret);
//    void* addr = (void*)shm.addr;


//    ret = open_shm(&shm2);
//    printf("open shm %d\n", ret);
//    void* addr2=(void*)shm2.addr;


//    getchar();
//    *(uint32_t*)addr = 4321;
//    printf("addr is %p  %u %d\n", addr,(*(uint32_t*)addr), shm.fd);
//    getchar();
//    printf("addr2 is %p  %u %d\n", addr2,*(uint32_t*)addr2, shm2.fd);
//    // = msync(addr, 4, MS_INVALIDATE|MS_SYNC);
//    ret = close_shm(&shm);
//    printf("ret is %d\n", ret);

//    printf("addr2 is %p  %u %d\n", addr2,*(uint32_t*)addr2, shm2.fd);
//    ret = close_shm(&shm2);
//    printf("ret is %d\n", ret);

//    ret = destroy_shm(&shm);
//    printf("destroy shm is %d\n", ret);
    */
#endif
    return 0;
}

