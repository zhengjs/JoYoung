#pragma once

#include "MovingTask_Base.h"
#include "MovingPlan_Edge.h"

/************************************************************************/
/* MovingTask_Edge_Rotate                                             */
/************************************************************************/
class MovingTask_Edge_Rotate :public MovingTask_Base
{
public:
    MovingTask_Edge_Rotate(MovingPlan* pParentPlan)
        : MovingTask_Base(pParentPlan)
    {
        memset(&m_nUltrasonicVariables, 0, sizeof(m_nUltrasonicVariables));
    }
    virtual ~MovingTask_Edge_Rotate(){ ; }

    virtual const std::string& taskName()override{ return Task_Edge_Rotate; }

    virtual void taskPlay()override;
    virtual void taskStop()override;
    virtual void taskPause()override;

    virtual void envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
        const LPVOID sensorData, const int sensorDataSize) override;

    virtual void envionmentVariables_Changed_vSlam()override{ ; }
protected:
    int m_nUltrasonicVariables[4];
};
