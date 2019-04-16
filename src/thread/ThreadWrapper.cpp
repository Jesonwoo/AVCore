#include <thread/ThreadWrapper.h>
#include <thread/IThread.h>
#include <thread/IMutex.h>
#include <common/typedefs.h>
#include <common/corelog.hpp>

void * CThreadWrapper::ThreadFunc(void * arg)
{
    CThreadWrapper * pWrapper = (CThreadWrapper *)arg;
    pWrapper->RunThread(pWrapper->m_pArgs);
    
    return NULL;
}
    
CThreadWrapper::CThreadWrapper(void)
    : m_pThread(NULL)
    , m_pLock(IMutex::Create())
    , m_bRunning(false)
    , m_pArgs(NULL)
{
}

CThreadWrapper::~CThreadWrapper(void)
{
    SAFE_DELETE(m_pThread);
    SAFE_DELETE(m_pLock);
    m_bRunning = false;
    m_pArgs = NULL;
}

int CThreadWrapper::StartThread(void * args, const char * name,
                                enThreadPriority priority)
{
    CMutexGuard guard(m_pLock);
    if (m_bRunning) {
        return 0;
    }
    
    m_bRunning = true;
    m_pArgs = args;
    m_pThread = IThread::Create(CThreadWrapper::ThreadFunc, this, name, priority);
    m_pThread->Start();
    
    return 0;
}

int CThreadWrapper::RunThread(void * args)
{
    LOGV("not implements RunThread\n");
    return 0;
}
    
int CThreadWrapper::StopThread(void)
{
    CMutexGuard guard(m_pLock);
    if (m_pThread != NULL) {
        m_pThread->Stop();
        SAFE_DELETE(m_pThread);
    }
    m_bRunning = false;
    m_pArgs = NULL;
    
    return 0;
}

bool CThreadWrapper::IsThreadRunning(void) const
{
    CMutexGuard guard(m_pLock);
    return m_bRunning;
}
