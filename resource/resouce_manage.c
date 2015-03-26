#include "eal/lmice_eal_common.h"
#include "eal/lmice_eal_thread.h"
#include "eal/lmice_eal_hash.h"
#include "eal/lmice_trace.h"

#include "resource_manage.h"
#include "rtspace.h"


#include <stdio.h>
#include <errno.h>

int create_server_resource(lm_resourece_t* server)
{
    int ret = 0;
    uint64_t hval = 0;
    pid_t m_pid = 0;
    lm_server_info_t *m_server = NULL;

    /*获取当前进程ID*/
    m_pid = getpid();

    /*创建平台公共区域共享内存*/
    server->shm.size = DEFAULT_SHM_SIZE * 2;
    hval = eal_hash64_fnv1a(BOARD_SHMNAME, sizeof(BOARD_SHMNAME)-1);
    eal_shm_hash_name(hval, server->shm.name);
    ret = eal_shm_create(&server->shm);
    if(ret != 0)
    {
        lmice_critical_print("create server shm failed[%d]\n", ret);
        return ret;
    }
    m_server = (lm_server_info_t*)((void*)(server->shm.addr));
    m_server->event_id = hval;
    m_server->lock = 0;
    m_server->size = server->shm.size;
    m_server->version = LMICE_VERSION;
    m_server->next_info_id = 0;

    /*创建公共区域管理事件*/
    eal_event_zero(&server->evt);
    eal_event_hash_name(hval, server->evt.name);
    ret = eal_event_create(&server->evt);
    if(ret != 0)
    {
        lmice_critical_print("create server event failed[%d]\n", ret);
    }
    return ret;
}

int destroy_server_resource(lm_resourece_t *server)
{
    int ret = 0;
    /* destroy client resource first */

    /* close server event and shm*/
    ret = eal_event_destroy(&server->evt);
    if(ret != 0)
        lmice_critical_print("destroy server event failed[%d]\n", ret);

    ret = eal_shm_destroy(&server->shm);
    if(ret != 0)
        lmice_critical_print("destroy server shm failed[%d]\n", ret);

    return ret;
}

//#include <jansson.h>

//const char* data_path = "data";

//int lmice_dump_resource_file()
//{

//    //TODO: server runtime --> instance scenario --> config --> data-path

//    //Dump scenario list json(id, type, owner)
//    const char* scen_temp = "\t{\n\t\tid:%lld\n\t\ttype:%lld\n\t\towner:%lld\n\t}\n";
//    char path[256]={0};
//    strcat(path, data_path);
//    strcat(path, "\\scenlist.log");
//    FILE* fp = fopen(path, "w");
//    if(!fp)
//        return errno;
//    setvbuf(fp, NULL, _IOFBF, 1024);
//    fwrite("[\n", 1, 2, fp);
//    fprintf(fp, scen_temp, 0, 1, 1);
//    fwrite("]\n", 1, 2, fp);
//    fflush(fp);
//    fclose(fp);

//    return 0;
//}

//int lmice_load_resource_file()
//{
//    char path[256]={0};
//    strcat(path, data_path);
//    strcat(path, "\\scenlist.log");

//    json_error_t err;
//    json_t *root;
//    FILE* fp = fopen(path, "r");
//    if(!fp)
//        return errno;

//    root = json_loadf(fp, 0, &err);

//    fclose(fp);

//}
