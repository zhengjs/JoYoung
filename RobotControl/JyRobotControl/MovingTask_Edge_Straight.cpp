#include "stdafx.h"
#include "MovingTask_Edge_Straight.h"
#include "MovingPlan_Base.h"

void MovingTask_Edge_Straight::taskPlay()
{
    JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
    if (!pRobot)
        return;
    pRobot->setMoveType(MT_Speed, 100, 100);
}

void MovingTask_Edge_Straight::taskStop()
{
}

void MovingTask_Edge_Straight::taskPause()
{
}

void MovingTask_Edge_Straight::envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
    const LPVOID sensorData, const int sensorDataSize)
{
//    if (TS_Playing == m_taskState)
//        return;
    if (ST_Ultrasonic != sensorType)
        return;
    if (sizeof(Variables_Ultrasonic) != sensorDataSize)
        return;
    Variables_Ultrasonic* ultrasonicVars = (Variables_Ultrasonic*)sensorData;
    do 
    {
        if (0 == ultrasonicVars->nIndex || 1 == ultrasonicVars->nIndex)
        {
            if (ultrasonicVars->nDistanceMM <= 240)
                break;
        }
        /*
        if (2 == ultrasonicVars->nIndex || 3 == ultrasonicVars->nIndex)
        {
            if (ultrasonicVars->nDistanceMM <= 320)
                break;
        }
        */
        return;
    }
    while (false);
    
    MovingPlanManager* pPlanManager = m_pPlanParent ? m_pPlanParent->planManager() : nullptr;
    if (!pPlanManager)
        return;
    JoyoungRobot* pRobot = pPlanManager->robot();
    if (!pRobot)
        return;
    pRobot->setMoveType(MT_Stop, 0, 0);

    ((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
}