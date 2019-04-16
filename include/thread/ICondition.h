
#ifndef __ICONDITION_H__
#define __ICONDITION_H__

#include <common/typedefs.h>

class IMutex;

class CORE_DEBUG_EXPORT ICondition
{
public:
    static ICondition * Create(IMutex * inputLock = NULL);

public:
    virtual ~ICondition(void);

    virtual void Sleep(void) = 0;

    virtual bool Sleep(int32_t maxms) = 0;

    virtual void Wake(void) = 0;

    virtual void WakeAll(void) = 0;
};

#endif
