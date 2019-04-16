#include <thread/IMutex.h>
#include <thread/MutexWin.h>

#include <assert.h>

IMutex * IMutex::Create(void)
{
    return new CMutexWin();
}

IMutex::~IMutex(void)
{
}

CMutexGuard::CMutexGuard(IMutex * mutex)
    : m_pMutex(mutex)
{
    assert(m_pMutex != NULL);
    m_pMutex->Lock();
}

CMutexGuard::CMutexGuard(IMutex & mutex)
    : m_pMutex(&mutex)
{
    m_pMutex->Lock();
}

CMutexGuard::~CMutexGuard(void)
{
    m_pMutex->Unlock();
    m_pMutex = NULL;
}
