#ifndef __IMUTEX_H__
#define __IMUTEX_H__

#include <common/typedefs.h>

class CORE_DEBUG_EXPORT IMutex
{
public:
    static IMutex * Create(void);

public:
    virtual ~IMutex(void);

    virtual void Lock(void) = 0;

    virtual void Unlock(void) = 0;
};

class CMutexGuard
{
public:
    CMutexGuard(IMutex * mutex);

    CMutexGuard(IMutex & mutex);

    ~CMutexGuard(void);

private:
    IMutex * m_pMutex;
};

#endif
