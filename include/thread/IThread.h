#ifndef __ITHREAD_H__
#define __ITHREAD_H__

#include <common/typedefs.h>

typedef void * ThreadObj;
typedef void * (*ThreadRunFunction)(ThreadObj);

enum enThreadPriority
{
    enLowPriority = 1,
    enNormalPriority = 2,
    enHighPriority = 3,
    enHighestPriority = 4,
    enRealtimePriority = 5,
};

enum
{
    enThreadMaxNameLength = 64
};

class CORE_DEBUG_EXPORT IThread
{
public:
    static IThread * Create(ThreadRunFunction func = NULL,
                            ThreadObj obj = NULL,
                            const char * name = NULL,
                            enThreadPriority priority = enNormalPriority);

    //curent thread sleep ms
    static int SleepMS(int ms);

public:
    IThread(void);

    virtual ~IThread(void);

    virtual int Start(void) = 0;

    virtual int Stop(void) = 0;

    virtual bool IsStarted() const;

protected:
    bool    m_bStarted;
};


#endif // __ITHREAD_H__
