#include "lmice_eal_event.h"

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
    memcpy(name, "Global\\", 7);
    hash_to_nameA(hval, name+7);
    name[23]='\0';
#else
    hash_to_nameA(hval, name);
    name[16]='\0';
#endif


    return 0;
}


#if defined(_WIN32)



#endif


int eal_event_create(lmice_event_t* e)
{
    e->fd = CreateEventA(
                NULL,
                TRUE,
                FALSE,
                e->name);
    if(e->fd == INVALID_HANDLE_VALUE)
    {
        e->fd = 0;
        return 1;
    }
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
