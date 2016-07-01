#pragma once
#include <string>
#include <atomic>
#include "MovingPlan.h"

/************************************************************************/
/* MovingTask_Base                                                      */
/************************************************************************/
enum Task_State :int
{
    TS_Stoped = 0,
    TS_Playing,
    TS_Pausing,
};

class MovingTask_Base :public MovingTask
{
public:
    MovingTask_Base(MovingPlan* pParentPlan)
        : MovingTask(pParentPlan)
        , m_pPlanParent(pParentPlan)
        , m_taskState(TS_Stoped)
    {
    }
    virtual ~MovingTask_Base(){ ; }

    virtual void taskPlay() = 0;
    virtual void taskStop() = 0;
    virtual void taskPause() = 0;

    virtual Task_State  taskState(){ return m_taskState; }

public:
    virtual void envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
        const LPVOID sensorData, const int sensorDataSize) = 0;

	virtual void sensorValuesChanged(SensorType sensorType) = 0;

protected:
    MovingPlan*                 m_pPlanParent;
    std::atomic<Task_State>     m_taskState;
};
