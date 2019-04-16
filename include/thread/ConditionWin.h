#ifndef _m_pCondition_WIN_H__
#define _m_pCondition_WIN_H__

#include <common/typedefs.h>
#include <thread/ICondition.h>

class IMutex;
class CConditionWin : public ICondition
{
public:
    CConditionWin(IMutex * inputLock = NULL);

    virtual ~CConditionWin(void);

    virtual void Sleep(void);

    virtual bool Sleep(int maxms);

    virtual void Wake(void);

    virtual void WakeAll(void);

private:
    enum enWakeType
    {
        enWakeAll0 = 0,
        enWakeAll1,
        enWakeOne,
        enWakeMax,
    };

private:
    CONDITION_VARIABLE  m_sCondition;
    IMutex *            m_pInputLock;
    IMutex *            m_pOwnerLock;

    HANDLE              m_pEvent[enWakeMax];
    int                 m_iWaiters[2];
    IMutex *            m_pWaiterLock;
};

#endif
