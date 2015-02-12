#ifndef LMICE_EAL_COMMON_H
#define LMICE_EAL_COMMON_H

/**
  support gcc msvc
*/
#if defined(__GNUC__)   /** GCC */

#define     forceinline __attribute__((always_inline))

#elif defined(_MSC_VER) /** MSC */

#define     forceinline __inline

#else                   /** Other compiler */

#error(Unsupported compiler)
#endif

#endif /** LMICE_EAL_COMMON_H */
