#ifndef _m_hThread_WRAPPER_H__
#define _m_hThread_WRAPPER_H__

#include <common/typedefs.h>
#include <thread/IThread.h>

class IThread;
class IMutex;

class CORE_DEBUG_EXPORT CThreadWrapper
{
public:
    static void * ThreadFunc(void * arg);

public:
    CThreadWrapper(void);

    virtual ~CThreadWrapper(void);

    virtual int StartThread(void * args, const char * name = "noname",
                            enThreadPriority priority = enNormalPriority);

    // By overwriting to implement the function what you want
    virtual int RunThread(void * args);

    virtual int StopThread(void);

    bool IsThreadRunning(void) const;

private:
    bool        m_bRunning;
    IThread *   m_pThread;
    void *      m_pArgs;
    IMutex *    m_pLock;
};

#endif
