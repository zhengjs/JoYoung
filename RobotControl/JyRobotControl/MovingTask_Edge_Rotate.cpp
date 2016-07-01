#include "stdafx.h"
#include "MovingTask_Edge_Rotate.h"
#include "MovingPlan_Base.h"

void MovingTask_Edge_Rotate::taskPlay()
{
    JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
    if (!pRobot)
        return;
	printf_s("current task: TASK_ROTATE\n");
    //pRobot->setMoveType(MT_Speed, 90, -90);//逆时针转动
    //pRobot->setMoveType(MT_Angle, 360, 10);//逆时针转动
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

void MovingTask_Edge_Rotate::sensorValuesChanged(SensorType sensorType){
	Sensor sensor = ((MovingPlan_Base*)m_pPlanParent)->m_sensor;

	if (!sensor.isRobotStopped())										// complete the last command
		return;
	JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
	if (sensor.mBump.leftBump){
		pRobot->setMoveType(MT_Distance, -100, 85);					// go backward 100mm at 85 mm/s(minimum speed)
		printf_s("TASK_ROTATE: Left bump! Go backward 100mm!\n");
		return;
	}
	if (sensor.mBump.rightBump){
		pRobot->setMoveType(MT_Distance, -100, 85);					// go backward 100mm at 85 mm/s
		printf_s("TASK_ROTATE: Right bump! Go backward 100mm!\n");
		return;
	}
	int angle = 0;
	if (sensor.mInfrared.infraredL1)
		angle = 30;
	if (sensor.mInfrared.infraredL2)
		angle = 60;
	if (sensor.mInfrared.infraredC)
		angle = 90;
	if (sensor.mInfrared.infraredR2)
		angle = 120;
	if (sensor.mInfrared.infraredR1)
		angle = 150;
	if (angle >= 30){
		pRobot->setMoveType(MT_Angle, angle, 60);
		printf_s("TASK_ROTATE: Turn right %d degrees!\n", angle);
		return;
	}
	pRobot->setMoveType(MT_Stop, 0, 0);
	((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
	printf_s("TASK_ROTATE FINISHED! No bump and no infrared. \n");
}