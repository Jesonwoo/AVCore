
#include <thread/ConditionWin.h>
#include <thread/MutexWin.h>
#include <common/corelog.hpp>
#include <common/typedefs.h>

typedef VOID (WINAPI *conditionm_func_t)(PCONDITION_VARIABLE ConditionVariable);
typedef BOOL (WINAPI *sleepm_func_t)(PCONDITION_VARIABLE ConditionVariable,
    PCRITICAL_SECTION CriticalSection, DWORD dwMilliseconds);

static bool gCheck                      = false;
static bool gSupportCondition           = false;

static conditionm_func_t pInitFunc       = NULL;
static conditionm_func_t pWakeFunc       = NULL;
static conditionm_func_t pWakeAllFunc    = NULL;
static sleepm_func_t     pSleepFunc      = NULL;

const static char * TAG = "condition";

CConditionWin::CConditionWin(IMutex * inputLock)
    : ICondition()
    , m_pInputLock(inputLock)	
    , m_pOwnerLock(NULL)
    , m_pWaiterLock(NULL)
{
    memset(m_pEvent, 0x00, ARRAYSIZE(m_pEvent));
    memset(m_iWaiters, 0x00, ARRAYSIZE(m_iWaiters));

    if (!gCheck) {
        gCheck = true;
        LOGI("load kernel32.dll and check condition support");
        
#ifdef _DEBUG
        HMODULE library = LoadLibrary("kernel32.dll");
#else
        HMODULE library = LoadLibrary(L"kernel32.dll");

#endif

        if (library != NULL) {
            pInitFunc = (conditionm_func_t)GetProcAddress(library, 
                "InitializeConditionVariable");
            pWakeFunc = (conditionm_func_t)GetProcAddress(library,
                "WakeConditionVariable");
            pWakeAllFunc = (conditionm_func_t)GetProcAddress(library,
                "WakeAllConditionVariable");
            pSleepFunc = (sleepm_func_t)GetProcAddress(library,
                "SleepConditionVariableCS");

            LOGI(TAG, "Init:%p, Wake:%p, WakeAll:%p, Sleep:%p\n",
                pInitFunc, pWakeFunc, pWakeAllFunc, pSleepFunc);

            if (pInitFunc != NULL && pWakeFunc != NULL &&
                pWakeAllFunc != NULL && pSleepFunc != NULL) {
                gSupportCondition = true;
            }
        }
    }

    LOGV(TAG, "support condition:%d\n", gSupportCondition);

    if (gSupportCondition) {
        pInitFunc(&m_sCondition);
    } else {
        m_pWaiterLock = IMutex::Create();
    }

    if (m_pInputLock == NULL) {
        m_pOwnerLock = IMutex::Create();
    }
}

CConditionWin::~CConditionWin(void)
{
    if (!gSupportCondition) {
        for (int i = 0; i < enWakeMax; i++) {
            if (m_pEvent[i] != NULL) {
                CloseHandle(m_pEvent[i]);
                m_pEvent[i] = NULL;
            }
        }
    }
    SAFE_DELETE(m_pOwnerLock);
    SAFE_DELETE(m_pWaiterLock);
}

void CConditionWin::Sleep(void)
{
    Sleep(INFINITE);
}

bool CConditionWin::Sleep(int maxms)
{
    IMutex * pLock = m_pInputLock == NULL ? m_pOwnerLock : m_pInputLock;
    CMutexWin * pWinLock = reinterpret_cast<CMutexWin *>(pLock);

    if (gSupportCondition) {
        BOOL ret = false;
        if (pLock == m_pInputLock) {
            ret = pSleepFunc(&m_sCondition, &pWinLock->m_tCS, maxms);
        } else {
            CMutexGuard guard(pLock);
            ret = pSleepFunc(&m_sCondition, &pWinLock->m_tCS, maxms);
        }
        return !!ret;
    }
    return true;
}

void CConditionWin::Wake(void)
{
    IMutex * pLock = m_pInputLock == NULL ? m_pOwnerLock : m_pInputLock;

    if (gSupportCondition) {
        if (pLock == m_pInputLock) {
            pWakeFunc(&m_sCondition);
        } else {
            CMutexGuard guard(pLock);
            pWakeFunc(&m_sCondition);
        }
    }
}

void CConditionWin::WakeAll(void)
{
    IMutex * pLock = m_pInputLock == NULL ? m_pOwnerLock : m_pInputLock;

    if (gSupportCondition) {
        if (pLock == m_pInputLock) {
            pWakeAllFunc(&m_sCondition);
        } else {
            CMutexGuard guard(pLock);
            pWakeAllFunc(&m_sCondition);
        }
    }
}
    
