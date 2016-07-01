#pragma once

#include "MovingTask_Base.h"
#include "MovingPlan_Edge.h"

/************************************************************************/
/* MovingTask_Edge_Along                                             */
/************************************************************************/
class MovingTask_Edge_Along :public MovingTask_Base
{
public:
    MovingTask_Edge_Along(MovingPlan* pParentPlan)
        : MovingTask_Base(pParentPlan)
    {
        memset(&m_nUltrasonicVariables, 0, sizeof(m_nUltrasonicVariables));
    }
    virtual ~MovingTask_Edge_Along(){ ; }

    virtual const std::string& taskName()override{ return Task_Edge_Along; }

    virtual void taskPlay()override;
    virtual void taskStop()override;
    virtual void taskPause()override;

    virtual void envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
        const LPVOID sensorData, const int sensorDataSize) override;

    virtual void envionmentVariables_Changed_vSlam()override{ ; }

	virtual void sensorValuesChanged(SensorType sensorType) override;

protected:
    int m_nUltrasonicVariables[4];

protected:
    bool checkUnsafe2Stop(JoyoungRobot* pRobot);
};
