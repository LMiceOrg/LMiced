#include "lmice_core.h"
#include "lmice_eal_endian.h"
#include "lmice_eal_common.h"
#include "lmice_trace.h"

/*
int lmice_process_cmdline(int argc, char * const *argv)
{
    return 0;
}


int lmice_exec_command(lmice_environment_t *env)
{
    return 0;
}
*/

#if defined(_WIN32)

#include <Iphlpapi.h>
/* Library iphlpapi.lib */
forceinline int get_net_bandwidth(uint32_t *net_bandwidth)
{
    DWORD dret = 0;
    DWORD buff_len = 0;
    IP_INTERFACE_INFO *iif = NULL;
    MIB_IFROW row;
    LONG i = 0;

    dret = GetInterfaceInfo(NULL, &buff_len);
    if(dret == ERROR_INSUFFICIENT_BUFFER) {
        iif = (PIP_INTERFACE_INFO)malloc(buff_len);
        GetInterfaceInfo(iif, &buff_len);
    } else {
        lmice_critical_print("get_net_bandwidth call GetInterfaceInfo failed[%u]\n", dret );
        return 1;
    }

    for(i=0; i< iif->NumAdapters; ++i) {

        row.dwIndex = iif->Adapter[i].Index;
        if(GetIfEntry(&row) == NO_ERROR) {
            /*
            _wprintf_p(L"Name: %s\t\n", row.wszName);
            printf("Desc:%s\n[%d] State:%u\tMtu:%u\tSpeed:%u\n\n", row.bDescr, iif->Adapter[i].Index,
                   row.dwOperStatus, row.dwMtu, row.dwSpeed);
            */
            *net_bandwidth = row.dwSpeed;
            break;
        }
    }
    free(iif);
    return 0;
}

forceinline void get_core_properties(uint32_t* lcore, uint32_t* mem, uint32_t* net_bandwidth)
{
    ULONGLONG memkilo = 0;
    SYSTEM_INFO mySysInfo;

    /* get lcore num */
    GetSystemInfo(&mySysInfo);
    *lcore = mySysInfo.dwNumberOfProcessors;

    /* get memory in Milibytes */
    GetPhysicallyInstalledSystemMemory(&memkilo);
    *mem = memkilo / 1024;

    /* get device [0] bandwidth */
    get_net_bandwidth(net_bandwidth);

}
#elif defined(__APPLE__)

#elif defined(__LINUX__)

#else

#error("No implementation of core functionalities.")
#endif


int eal_env_init(lmice_environment_t* env)
{
    get_core_properties( &(env->lcore), &(env->memory), &(env->net_bandwidth) );

    return 0;
}
