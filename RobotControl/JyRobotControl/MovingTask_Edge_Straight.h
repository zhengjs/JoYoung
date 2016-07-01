#pragma once

#include "MovingTask_Base.h"
#include "MovingPlan_Edge.h"

/************************************************************************/
/* MovingTask_Edge_Straight                                             */
/************************************************************************/
class MovingTask_Edge_Straight :public MovingTask_Base
{
public:
    MovingTask_Edge_Straight(MovingPlan* pParentPlan)
        : MovingTask_Base(pParentPlan)
    {
    }
    virtual ~MovingTask_Edge_Straight(){ ; }

    virtual const std::string& taskName()override{ return Task_Edge_Straight; }

    virtual void taskPlay()override;
    virtual void taskStop()override;
    virtual void taskPause()override;

    virtual void envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
        const LPVOID sensorData, const int sensorDataSize) override;

    virtual void envionmentVariables_Changed_vSlam()override{ ; }

	virtual void sensorValuesChanged(SensorType sensorType) override;

};
