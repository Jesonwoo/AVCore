#ifndef __LOG_H__
#define __LOG_H__

#include <common/typedefs.h>
#include <stdarg.h>
#include <string.h>
//#define SHOW_LINE_INFO

#if defined(LOG_TID)
#include <windows.h>
#endif
enum {
    LogUnknown = 0,
    LogDefault,
    LogVerbose,
    LogDebug,
    LogInfo,
    LogWarn,
    LogError,
    LogFatal,
    LogSilent,
};


#ifdef __cplusplus
extern "C" {
#endif
    typedef void (*CoreLogType)(int, const char *, const char *, va_list);
    
    CORE_EXPORT void SetCoreLogLevel(int level);
    
    CORE_EXPORT void SetCoreLog(CoreLogType log);

    CORE_EXPORT void CoreLog(int level, const char * tag, const char * fmt, ...);
    
#ifdef __cplusplus
}
#endif

void LOG(int level, const char * tag, const char * fmt, ...);

#if defined(SHOW_LINE_INFO)
#define LOGV(tag, fmt, ...)   \
    {const char * __i__ = strrchr(__FILE__, '\\');  \
        sdk_log(log_verbose, tag, "{%s %s:%d} " fmt, \
        __i__ ?  __i__ + 1 : __FILE__, \
        __FUNCTION__, __LINE__, __VA_ARGS__); }

#define LOGD(tag, fmt, ...)   \
    {const char * __i__ = strrchr(__FILE__, '\\');  \
        sdk_log(log_debug, tag, "{%s %s:%d} " fmt, \
        __i__ ?  __i__ + 1 : __FILE__, \
        __FUNCTION__, __LINE__, __VA_ARGS__); }

#define LOGI(tag, fmt, ...)   \
    {const char * __i__ = strrchr(__FILE__, '\\');  \
        sdk_log(log_info, tag, "{%s %s:%d} " fmt, \
        __i__ ?  __i__ + 1 : __FILE__, \
        __FUNCTION__, __LINE__, __VA_ARGS__); }

#define LOGW(tag, fmt, ...)   \
    {const char * __i__ = strrchr(__FILE__, '\\');  \
        sdk_log(log_warn, tag, "{%s %s:%d} " fmt, \
        __i__ ?  __i__ + 1 : __FILE__, \
        __FUNCTION__, __LINE__, __VA_ARGS__); }

#define LOGE(tag, fmt, ...)   \
    {const char * __i__ = strrchr(__FILE__, '\\');  \
        sdk_log(log_error, tag, "{%s %s:%d} " fmt, \
        __i__ ?  __i__ + 1 : __FILE__, \
        __FUNCTION__, __LINE__, __VA_ARGS__); }

#define LOGF(tag, fmt, ...)   \
    {const char * __i__ = strrchr(__FILE__, '\\');  \
        sdk_log(log_fatal, tag, "{%s %s:%d} " fmt, \
        __i__ ?  __i__ + 1 : __FILE__, \
        __FUNCTION__, __LINE__, __VA_ARGS__); }
#else
#define LOGV(tag, fmt, ...)     CoreLog(LogVerbose, tag, fmt, __VA_ARGS__)
#define LOGD(tag, fmt, ...)     CoreLog(LogDebug, tag, fmt, __VA_ARGS__)
#define LOGI(tag, fmt, ...)     CoreLog(LogInfo, tag, fmt, __VA_ARGS__)
#define LOGW(tag, fmt, ...)     CoreLog(LogWarn, tag, fmt, __VA_ARGS__)
#define LOGE(tag, fmt, ...)     CoreLog(LogError, tag, fmt, __VA_ARGS__)
#define LOGF(tag, fmt, ...)     CoreLog(LogFatal, tag, fmt, __VA_ARGS__)
#endif // !SHOW_LINE_INFO

#endif // __LOG_H__g
