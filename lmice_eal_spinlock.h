#ifndef LMICE_EAL_SPINLOCK_H
#define LMICE_EAL_SPINLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int eal_spin_trylock(uint64_t* lock);
int eal_spin_lock(uint64_t* lock);
int eal_spin_unlock(uint64_t* lock);

#ifdef __cplusplus
}
#endif
#endif /** LMICE_EAL_SPINLOCK_H */
