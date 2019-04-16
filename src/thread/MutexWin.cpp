#include <thread/MutexWin.h>

CMutexWin::CMutexWin(void)
{
    InitializeCriticalSection(&m_tCS);
}

CMutexWin::~CMutexWin(void)
{
    DeleteCriticalSection(&m_tCS);
}

void CMutexWin::Lock(void)
{
    EnterCriticalSection(&m_tCS);
}

void CMutexWin::Unlock(void)
{
    LeaveCriticalSection(&m_tCS);
}
