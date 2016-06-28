#pragma once

#include <string>
#include <atomic>

#include "MovingPlan.h"

/************************************************************************/
/* MovingPlan_Base                                                      */
/************************************************************************/
class MovingTask_Base;

class MovingPlan_Base :public MovingPlan
{
public:
    MovingPlan_Base(MovingPlanManager* pManager)
        :MovingPlan(pManager)
        , m_pPlanManager(pManager)
        , m_pTaskCurrent(nullptr)
    {}
    virtual ~MovingPlan_Base(){ ; }

public:
    virtual void taskFinished(MovingTask* pTask, LPVOID taskFinishedReturnData, int returnDataSize) = 0;

protected:
    virtual MovingTask* taskCreate(const std::string& taskName) = 0;

protected:
    MovingPlanManager*              m_pPlanManager;
    std::atomic<MovingTask_Base*>   m_pTaskCurrent;
};
