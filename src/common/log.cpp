#include <common/log.h>

#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>

#ifdef __cplusplus
extern "C" {
#endif
//#include <3rd/ffmpeg/libavutil/log.h>
#ifdef __cplusplus
}
#endif

static const char level_char[] = {'U', 'D', 'V', 'D', 'I', 'W', 'E', 'F', 'S'};

static void DefLog(int level, const char * tag, const char *fmt, va_list valist);

static CoreLogType glog   = DefLog;
static int glevel         = LogInfo;

void DefLog(int level, const char * tag, const char *fmt, va_list valist)
{
    if (level < glevel) {
        return;
    }
    
    char line[4096];
    const int linesize = 4096;
    
    int off = 0;
    time_t t_time = time(NULL);

    struct tm tm_time;
    localtime_s(&tm_time, &t_time);

    off += strftime(line + off, linesize - off, "[%Y-%m-%d %H:%M:%S]", &tm_time);
    
#if defined(LOG_TID)
    off += snprintf(line + off, linesize - off, "[P:%zu]",
                    (size_t)GetCurrentThreadId());
#endif
    
    off += snprintf(line + off, linesize - off, " %c/%s: ",
                    level_char[level], tag);

    off += vsnprintf(line + off, linesize - off, fmt, valist);
    
    write(1, line, off);
    
    return;
}

CORE_EXPORT void SetCoreLogLevel(int level)
{
    if (level >= LogVerbose && level <= LogFatal) {
        glevel = level;
    }
}

CORE_EXPORT void SetCoreLog(CoreLogType log)
{
    glog = log;
}

CORE_EXPORT void CoreLog(int level, const char * tag, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    glog(level, tag, fmt, args);
    va_end(args);
}
