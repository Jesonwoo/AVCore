#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#include <stdio.h>          //define NULL.

// Include winsock2.h before including <windows.h> to maintain consistency with
// win32.h.
#include <winsock2.h>   //SOCKET
#include <windows.h>

#if _MSC_VER >= 1900
#define PI32        "zd"
#define PI64        "lld"
#define PU64        "llu"
#define snprintf    snprintf
#else
#define PI32        "d"
#define PI64        "I64d"
#define PU64        "I64u"
#define snprintf    _snprintf
#endif

#pragma  warning(disable:4996)
#pragma  warning(disable:4068)
#pragma  warning(disable:4146)
#pragma  warning(disable:4819)

#define CORE_EXPORT          __declspec(dllexport)
#if _DEBUG
#define CORE_DEBUG_EXPORT    __declspec(dllexport)
#endif

#ifndef INT64_MAX
#define INT64_MAX                   0x7fffffffffffffffLL
#endif

#ifndef INT32_MAX
#define INT32_MAX                   0x7fffffff
#endif

#endif // __TYPEDEF_H__
