#include <common/corelog.hpp>
#include <thread/IThread.h>
#include <thread/ThreadWin.h>

IThread * IThread::Create(ThreadRunFunction func, ThreadObj obj,
    const char * name, enThreadPriority priority)
{
    return new CThreadWin(func, obj, priority, name);
}

int IThread::SleepMS(int ms)
{

    Sleep(ms);
    return 0;
}

IThread::IThread(void)
    : m_bStarted(false)
{
}

IThread::~IThread(void)
{
}

bool IThread::IsStarted() const
{
    return m_bStarted;
}

