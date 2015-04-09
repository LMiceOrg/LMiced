#ifndef LMICE_EAL_COMMON_H
#define LMICE_EAL_COMMON_H

#define UNREFERENCED_PARAM(param) (void)param
/**
  support gcc msvc
*/
#if defined(__GNUC__)   /** GCC */

#define     forceinline __attribute__((always_inline))

#if defined(__MINGW32__)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#endif

#elif defined(_MSC_VER) /** MSC */

#define     forceinline __inline
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>


#else                   /** Other compiler */

#error(Unsupported compiler)
#endif

#endif /** LMICE_EAL_COMMON_H */
