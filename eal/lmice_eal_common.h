#ifndef LMICE_EAL_COMMON_H
#define LMICE_EAL_COMMON_H

#define UNREFERENCED_PARAM(param) (void)param
/**
  support gcc msvc
*/
#if defined(__GNUC__)   /** GCC */
#include <unistd.h>
#define forceinline __attribute__((always_inline)) inline static
#define forcepack(x) __attribute__((__aligned__((x))))
#include "sys/types.h"

#if defined(__MINGW32__)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#endif

#elif defined(_MSC_VER) /** MSC */

#define forceinline static __forceinline
#define forcepack(x) __declspec(align(x))
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#else                   /** Other compiler */

#error(Unsupported compiler)
#endif

#endif /** LMICE_EAL_COMMON_H */
