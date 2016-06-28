#include "stdafx.h"
#include "MovingTask_Edge_Rotate.h"
#include "MovingPlan_Base.h"

void MovingTask_Edge_Rotate::taskPlay()
{
    JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
    if (!pRobot)
        return;
    pRobot->setMoveType(MT_Speed, 90, -90);//逆时针转动
//    pRobot->setMoveType(MT_Angle, 360, 10);//逆时针转动
}

void MovingTask_Edge_Rotate::taskStop()
{
}

void MovingTask_Edge_Rotate::taskPause()
{
}

void MovingTask_Edge_Rotate::envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
    const LPVOID sensorData, const int sensorDataSize)
{
    //    if (TS_Playing == m_taskState)
    //        return;
    if (ST_Ultrasonic != sensorType)
        return;
    if (sizeof(Variables_Ultrasonic) != sensorDataSize)
        return;

    Variables_Ultrasonic* ultrasonicVars = (Variables_Ultrasonic*)sensorData;
    m_nUltrasonicVariables[ultrasonicVars->nIndex] = ultrasonicVars->nDistanceMM;

    MovingPlanManager* pPlanManager = m_pPlanParent ? m_pPlanParent->planManager() : nullptr;
    if (!pPlanManager)
        return;
    JoyoungRobot* pRobot = pPlanManager->robot();
    if (!pRobot)
        return;

    do{
        if (/*m_nUltrasonicVariables[0] <400x ||*/ m_nUltrasonicVariables[1] <400)
            break;

        if (m_nUltrasonicVariables[2] > 600 || m_nUltrasonicVariables[3] >600)
            break;

        if (abs(m_nUltrasonicVariables[2] - m_nUltrasonicVariables[3]) >10)
            break;

        pRobot->setMoveType(MT_Stop, 0, 0);
        ((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
    }
    while (false);
}