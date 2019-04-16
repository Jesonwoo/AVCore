#ifndef __MUTEX_WIN_H__
#define __MUTEX_WIN_H__

#include <thread/IMutex.h>
#include <windows.h>

class CMutexWin : public IMutex
{
public:
    CMutexWin(void);
    
    virtual ~CMutexWin(void);
    
    virtual void Lock(void);
    
    virtual void Unlock(void);
    
private:
    CRITICAL_SECTION m_tCS;

    friend class CConditionWin;
};

#endif
