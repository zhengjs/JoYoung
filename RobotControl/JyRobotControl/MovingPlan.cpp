#include "stdafx.h"
#include "MovingTask_Base.h"
#include "sensor.h"
#include "MovingPlan_Edge.h"

/************************************************************************/
/* MovingPlanManager                                                      */
/************************************************************************/

MovingPlanManager::MovingPlanManager(JoyoungRobot* pRobot)
: m_pRobot(pRobot)
, m_planCurrent(nullptr)
{

}

MovingPlanManager::~MovingPlanManager()
{

}

MovingPlan* MovingPlanManager::planCurrent()
{
    return m_planCurrent;
}

MovingTask* MovingPlanManager::taskCurrent()
{
    if (nullptr ==(MovingPlan*)m_planCurrent)
        return nullptr;
    return ((MovingPlan*)m_planCurrent)->taskCurrent();
}

MovingPlan* MovingPlanManager::planLaunch(const std::string& planName)
{
    SafeDeleteAtomic(MovingPlan*, m_planCurrent);

    if (0 == planName.compare(Plan_Edge))
    {
        m_planCurrent = new MovingPlan_Edge(this, robotSensor);
    }
    if ((MovingPlan*)m_planCurrent)
        ((MovingPlan*)m_planCurrent)->planPlay();
    return m_planCurrent;
}

const std::string MovingPlanManager::Plan_Edge = "Plan_Edge";