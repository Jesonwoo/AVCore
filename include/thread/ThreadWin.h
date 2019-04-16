#ifndef _m_hThread_WIN_H__
#define _m_hThread_WIN_H__

#include <thread/IThread.h>
class IMutex;
class ICondition;

class CThreadWin : public IThread
{
public:
    CThreadWin(ThreadRunFunction func, ThreadObj obj,
               enThreadPriority priority, const char * name);

    virtual ~CThreadWin(void);

    virtual int Start(void);

    virtual int Stop(void);

public:
    static unsigned __stdcall ThreadFunc(void * arg);

private:
    void Run(void);

    const char * Name() const;

private:
    ThreadRunFunction   m_func;
    ThreadObj           m_threadObj;
    enThreadPriority    m_priority;

    bool                m_bAlive;
    bool                m_bDead;
    bool                m_bStart;

    bool                m_bNamed;
    char                m_name[enThreadMaxNameLength];

    HANDLE              m_hThread;
    unsigned            m_iTid;

    IMutex *            m_pLock;
    ICondition *        m_pCondition;
};

#endif
