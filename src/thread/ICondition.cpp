#include <thread/ICondition.h>
#include <thread/ConditionWin.h>

ICondition * ICondition::Create(IMutex * inputLock/* = NULL*/)
{
    return new CConditionWin(inputLock);
}

ICondition::~ICondition(void)
{
    
}

