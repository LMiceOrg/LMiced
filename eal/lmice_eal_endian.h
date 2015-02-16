#ifndef LMICE_EAL_ENDIAN_H
#define LMICE_EAL_ENDIAN_H

#include "lmice_eal_common.h"

static int forceinline
eal_is_little_endian()
{
    unsigned short ed = 0x0001;
    return (char)ed;
}

static int forceinline
eal_is_big_endian()
{
    unsigned short ed = 0x0100;

    return (char)ed;
}


#endif /** LMICE_EAL_ENDIAN_H */
