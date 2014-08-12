#ifndef LMICE_EAL_ENDIAN_H
#define LMICE_EAL_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

static inline int __attribute__((always_inline))
eal_is_little_endian()
{
    unsigned short ed = 0x0001;
    return (char)ed;
}

static inline int __attribute__((always_inline))
eal_is_big_endian()
{
    unsigned short ed = 0x0100;

    return (char)ed;
}

#ifdef __cplusplus
}
#endif

#endif /* LMICE_EAL_ENDIAN_H */
