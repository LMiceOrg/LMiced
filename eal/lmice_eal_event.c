#include "lmice_eal_event.h"
#include "lmice_trace.h"

#include <string.h>

static forceinline
void hash_to_nameA(uint64_t hval, char* name)
{
    const char* hex_list="0123456789ABCDEF";
    for(int i=0; i<8; ++i)
    {
        name[i*2] = hex_list[ *( (uint8_t*)&hval+i) >> 4];
        name[i*2+1] = hex_list[ *( (uint8_t*)&hval+i) & 0xf ];
    }
}

int eal_event_hash_name(uint64_t hval, char *name)
{
#if defined(_WIN32)
    memcpy(name, "Global\\ev", 9);
    hash_to_nameA(hval, name+9);
    name[25]='\0';
#else
    memcpy(name, "ev", 2);
    hash_to_nameA(hval, name+2);
    name[18]='\0';
#endif


    return 0;
}


#if defined(_WIN32)


int eal_event_create(lmice_event_t* e)
{
    e->fd = CreateEventA(
                NULL,
                FALSE,
                FALSE,
                e->name);
    if(e->fd == NULL)
    {
        DWORD hr = GetLastError();
        lmice_error_print("Create event[%s] failed[%u]", e->name, hr);
        e->fd = 0;
        return 1;
    }
    lmice_debug_print("event[%s] created as[%d]", e->name, (uint64_t)e->fd);
    return 0;
}


int eal_event_zero(lmice_event_t *e)
{
    memset(e, 0, sizeof(lmice_event_t));
    return 0;
}


int eal_event_awake(uint64_t eid)
{
    return SetEvent((HANDLE)eid);
}

#elif defined(__APPLE__) || defined(__LINUX__)

int eal_event_create(lmice_event_t* e)
{

}

#endif
