#include <thread/ThreadWin.h>
#include <thread/MutexWin.h>
#include <thread/ConditionWin.h>
#include <common/corelog.hpp>
#include <common/typedefs.h>

#include <process.h>

static const DWORD kVCThreadNameException = 0x406D1388;

static void SetThreadName(DWORD threadID, LPCSTR szname)
{
    struct {
        DWORD   type;
        LPCSTR  name;
        DWORD   id;
        DWORD   flags;
    } info = {
        0x1000, szname, threadID, 0,
    };

    __try {
        RaiseException(
            kVCThreadNameException, //0x406D1388,
            0,
            sizeof(info) / sizeof(DWORD), 
            reinterpret_cast<DWORD_PTR*>(&info));
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
        LOGE("set thread name<%s> exception", szname);
    }
}

CThreadWin::CThreadWin(ThreadRunFunction func, ThreadObj obj,
    enThreadPriority priority, const char * name)
    : IThread()
    , m_func(func)
    , m_threadObj(obj)
    , m_priority(priority)
    , m_bAlive(false)
    , m_bDead(false)
    , m_bStart(false)
    , m_bNamed(false)
    , m_hThread(NULL)
    , m_pLock(IMutex::Create())
    , m_pCondition(ICondition::Create(m_pLock))
{
    memset(m_name, 0, ARRAYSIZE(m_name));
    if (name != NULL) {
        m_bNamed = true;

        if (strlen(name) >= enThreadMaxNameLength) {
            strncpy(m_name, name, enThreadMaxNameLength);
            m_name[enThreadMaxNameLength - 1] = '\0';
        } else {
            strcpy(m_name, name);
        }
    } else {
        strcpy(m_name, "unknown");
    }
    LOGV("thread(%s) create", Name());
}

CThreadWin::~CThreadWin(void)
{
    LOGV("thread(%s) destroy", Name());

    if (m_hThread != NULL) {
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
    SAFE_DELETE(m_pCondition);
    SAFE_DELETE(m_pLock);
}

int CThreadWin::Start(void)
{
    unsigned id = 0;
    LOGV("thread(%s) begin start", Name());

    m_hThread = (HANDLE)_beginthreadex(NULL,
        0/*1024 * 1024*/, &CThreadWin::ThreadFunc, (void *)this, 0, &id);

    if (m_hThread == NULL) {
        LOGE("thread(%s) _beginthreadex error");
        return -1;
    }
    m_iTid = id;
    m_bStarted = true;

    {
        CMutexGuard guard(m_pLock);
        LOGV("thread(%s) start state:%d", Name(), m_bStart);
        if (!m_bStart) {
            if (!m_pCondition->Sleep(10000)) {
                m_func = NULL;
                LOGE("thread(%s) wa9t start state error", Name());
                return -2;
            }
        }
        LOGV("thread(%s) start state:%d", Name(), m_bStart);
    }

    if (m_priority) {
        switch (m_priority) {
        case enLowPriority:
            SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
            break;
        case enNormalPriority:
            SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);
            break;
        case enHighPriority:
            SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
            break;
        case enHighestPriority:
            SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
            break;
        case enRealtimePriority:
            SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);
            break;
        }
    }

    return 0;
}

unsigned __stdcall CThreadWin::ThreadFunc(void * arg)
{
    CThreadWin * pArg = static_cast<CThreadWin *>(arg);
    pArg->Run();

    return 0;
}

int CThreadWin::Stop(void)
{
    LOGV("thread(%s) entrance", Name());

    bool dead = false;
    {
        CMutexGuard guard(m_pLock);
        LOGV("thread(%s) alive:%d, dead:%d", Name(), m_bAlive, m_bDead);

        m_bAlive = false;
        if (m_bStart && !m_bDead) {
            m_pCondition->Sleep();
        }
        dead = m_bDead;
    }
    LOGV("thread(%s) exit", Name());
    if (dead) {
        m_bStarted = false;
    }

    return 0;
}

void CThreadWin::Run(void)
{
    LOGV("thread(%s) running entrance", Name());
    {
        CMutexGuard guard(m_pLock);
        m_bAlive = true;
        m_bStart = true;
        m_bDead  = false;

        LOGI("thread(%s) set state, and running", Name());
        m_pCondition->WakeAll();
    }

    bool alive = false;
    do {
        if (m_func != NULL) {
            if (m_func(m_threadObj) == NULL) {
                alive = false;
            }
        } else {
            CMutexGuard guard(m_pLock);
            if (!alive) {
                m_bAlive = alive;
            }
            alive = m_bAlive;
        }
    } while (alive);
    LOGV("thread(%s) running exit", Name());

    {
        CMutexGuard guard(m_pLock);
        m_bDead = true;
        m_pCondition->WakeAll();
    }
}

const char * CThreadWin::Name() const
{
    return m_name;
}
