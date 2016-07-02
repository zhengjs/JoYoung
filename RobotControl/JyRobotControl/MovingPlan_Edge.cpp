#include "stdafx.h"
#include "MovingPlan_Edge.h"

#include "MovingTask_Edge_Straight.h"
#include "MovingTask_Edge_Rotate.h"
#include "MovingTask_Edge_Along.h"

MovingPlan_Edge::MovingPlan_Edge(MovingPlanManager* pPlanManager, Sensor& sensor_)
: MovingPlan_Base(pPlanManager, sensor_)
, m_nTestAlongNum(0)
{
    m_pTaskCurrent = (MovingTask_Base*)taskCreate(Edge_FirstTask);
}

MovingPlan_Edge::~MovingPlan_Edge()
{
}

MovingTask* MovingPlan_Edge::taskCurrent()
{
    return (MovingTask*)m_pTaskCurrent;
}

MovingTask* MovingPlan_Edge::taskCreate(const std::string& taskName)
{
    if (0 == taskName.compare(Task_Edge_Straight))
    {
        return new MovingTask_Edge_Straight(this);
    }
    else if (0 == taskName.compare(Task_Edge_Rotate))
    {
        return new MovingTask_Edge_Rotate(this);
    }
    else if (0 == taskName.compare(Task_Edge_Along))
    {
        return new MovingTask_Edge_Along(this);
    }
    return nullptr;
}


void MovingPlan_Edge::planPlay()
{
    if (nullptr == (MovingTask_Base*)m_pTaskCurrent)
    {
        m_pTaskCurrent = (MovingTask_Base*)taskCreate(Edge_FirstTask);
    }
    ((MovingTask_Base*)m_pTaskCurrent)->taskPlay();
}

void MovingPlan_Edge::planStop()
{
    m_pTaskCurrent = nullptr;
}

void MovingPlan_Edge::planPause()
{
}

void MovingPlan_Edge::taskFinished(MovingTask* pTask, LPVOID taskFinishedReturnData, int returnDataSize)
{
    if (!pTask || pTask != (MovingTask_Base*)m_pTaskCurrent)
        return;

    std::string taskName = pTask->taskName();
    m_pTaskCurrent = nullptr;

    if (0 == taskName.compare(Task_Edge_Straight))
        m_pTaskCurrent = (MovingTask_Base*)taskCreate(Task_Edge_Rotate);
    else if (0 == taskName.compare(Task_Edge_Rotate))
        m_pTaskCurrent = (MovingTask_Base*)taskCreate(Task_Edge_Along);
    else if (0 == taskName.compare(Task_Edge_Along))
    {
        if (m_nTestAlongNum++ < 3)
            m_pTaskCurrent = (MovingTask_Base*)taskCreate(Task_Edge_Rotate);
    }

    if ((MovingTask_Base*)m_pTaskCurrent)
    {
        ((MovingTask_Base*)m_pTaskCurrent)->taskPlay();
    }
}
