#pragma once
#include <string>
#include <atomic>
#include "MovingPlan_Base.h"

/************************************************************************/
/* MovingPlan_Edge                                                      */
/************************************************************************/
class MovingPlan_Edge :public MovingPlan_Base
{
public:
	MovingPlan_Edge(MovingPlanManager* pManager, Sensor& sensor_);
    ~MovingPlan_Edge();

    virtual const std::string& planName()override{ return MovingPlanManager::Plan_Edge; }

    virtual MovingPlanManager* planManager()override{ return m_pPlanManager; }

    virtual void planPlay()override;
    virtual void planStop()override;
    virtual void planPause()override;

    virtual void taskFinished(MovingTask* pTask, LPVOID taskFinishedReturnData, int returnDataSize)override;

    MovingTask* taskCurrent()override;

protected:
    virtual MovingTask* taskCreate(const std::string& taskName)override;
protected:
    int m_nTestAlongNum;
};

const static std::string Task_Edge_Straight = "Task_Edge_Straight";
const static std::string Task_Edge_Rotate   = "Task_Edge_Rotate";
const static std::string Task_Edge_Along    = "Task_Edge_Along";

const static std::string Edge_FirstTask = Task_Edge_Along;//Task_Edge_Straight;
