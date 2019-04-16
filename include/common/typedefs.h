#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#include <stdio.h>            //define NULL.

// Include winsock2.h before including <windows.h> to maintain consistency with
// win32.h.
#include <winsock2.h>   //SOCKET
#include <windows.h>

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed __int64      int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned __int64    uint64_t;

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
#elif
#define CORE_DEBUG_EXPORT    
#endif

#define SAFE_DELETE(ptr)        \
    do {                        \
        if (ptr != NULL) {      \
            delete ptr;         \
            ptr = NULL;         \
        }                       \
    } while (0)

#define SAFE_FREE(ptr)          \
    do {                        \
        if (ptr != NULL) {      \
            free(ptr);          \
            ptr = NULL;         \
        }                       \
    } while (0)

#endif // __TYPEDEF_H__
