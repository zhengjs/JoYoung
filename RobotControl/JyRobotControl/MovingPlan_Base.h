#pragma once

#include <string>
#include <atomic>

#include "MovingPlan.h"
#include "sensor.h"

/************************************************************************/
/* MovingPlan_Base                                                      */
/************************************************************************/
class MovingTask_Base;

class MovingPlan_Base :public MovingPlan
{
public:
    MovingPlan_Base(MovingPlanManager* pManager, Sensor& sensor_)
        :MovingPlan(pManager)
        , m_pPlanManager(pManager)
        , m_pTaskCurrent(nullptr)
		, m_sensor(sensor_)
    {}
	virtual ~MovingPlan_Base(){ ; }
	Sensor&					m_sensor;

public:
    virtual void taskFinished(MovingTask* pTask, LPVOID taskFinishedReturnData, int returnDataSize) = 0;

protected:
    virtual MovingTask* taskCreate(const std::string& taskName) = 0;

protected:
    MovingPlanManager*              m_pPlanManager;
    std::atomic<MovingTask_Base*>   m_pTaskCurrent;
};
