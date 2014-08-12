#ifndef LMICE_EAL_ATOMIC_H
#define LMICE_EAL_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && __GNUC__ > 3
    #define eal_fetch_and_add32(ptr, value) __sync_fetch_and_add(ptr, value)
    #define eal_fetch_and_add64(ptr, value) __sync_fetch_and_add(ptr, value)
    #define eal_fetch_and_sub32(ptr, value) __sync_fetch_and_sub(ptr, value)
    #define eal_fetch_and_sub64(ptr, value) __sync_fetch_and_sub(ptr, value)
    #define eal_fetch_and_or32(ptr, value)  __sync_fetch_and_or(ptr, value)
    #define eal_fetch_and_or64(ptr, value)  __sync_fetch_and_or(ptr, value)
    #define eal_fetch_and_xor32(ptr, value) __sync_fetch_and_xor(ptr, value)
    #define eal_fetch_and_xor64(ptr, value) __sync_fetch_and_xor(ptr, value)

    #define eal_add_and_fetch32(ptr, value) __sync_add_and_fetch(ptr, value)
    #define eal_add_and_fetch64(ptr, value) __sync_add_and_fetch(ptr, value)
    #define eal_sub_and_fetch32(ptr, value) __sync_sub_and_fetch(ptr, value)
    #define eal_sub_and_fetch64(ptr, value) __sync_sub_and_fetch(ptr, value)
    #define eal_or_and_fetch32(ptr, value)  __sync_or_and_fetch(ptr, value)
    #define eal_or_and_fetch64(ptr, value)  __sync_or_and_fetch(ptr, value)
    #define eal_xor_and_fetch32(ptr, value) __sync_xor_and_fetch(ptr, value)
    #define eal_xor_and_fetch64(ptr, value) __sync_xor_and_fetch(ptr, value)

    #define eal_bool_compare_and_swap(ptr, oldval, newval) __sync_bool_compare_and_swap(ptr, oldval, newval)
    #define eal_val_compare_and_swap(ptr, oldval, newval) __sync_val_compare_and_swap(ptr, oldval, newval)

    #define eal_synchronize() __sync_synchronize()
#else
    #error(No atomic implementation!)
#endif

#ifdef __cplusplus
}
#endif

#endif /** LMICE_EAL_ATOMIC_H */
