#include "stdafx.h"
#include "MovingTask_Edge_Rotate.h"
#include "MovingPlan_Base.h"
#include <math.h>

void MovingTask_Edge_Rotate::taskPlay()
{
    JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
    if (!pRobot)
        return;
	printf_s("current task: TASK_ROTATE\n");
    //pRobot->setMoveType(MT_Speed, 90, -90);	//˳ʱ��ת��
    //pRobot->setMoveType(MT_Angle, 90, 60);		//rotate 90 degrees clockwise, when use ultrasonic
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
	if (false)
    do{
        if (/*m_nUltrasonicVariables[0] <400x ||*/ m_nUltrasonicVariables[1] <400)
            break;

        if (m_nUltrasonicVariables[2] > 600 || m_nUltrasonicVariables[3] >600)
            break;

        if (abs(m_nUltrasonicVariables[2] - m_nUltrasonicVariables[3]) >50)
            break;

		pRobot->setMoveType(MT_Stop, 0, 0);
		((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
    }
    while (false);

	do{
		Sensor sensor = ((MovingPlan_Base*)m_pPlanParent)->m_sensor;
		if (!sensor.isRobotStopped())									//wait until robot stop
			break;
		if (m_nUltrasonicVariables[2] < 1000 && m_nUltrasonicVariables[3] < 1000){
			int deltaD = m_nUltrasonicVariables[2] - m_nUltrasonicVariables[3];
			float angle = atan2f(deltaD, DIST_ULTRASONIC23);
			angle = angle*180.0 / 3.1415926;
			if (abs(angle)>10){
				pRobot->setMoveType(MT_Angle, -angle, 30);
				break;
			}
		}
		else
		{
			//TODO:	
		}
		pRobot->setMoveType(MT_Stop, 0, 0);
		((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
	} while (false);
}

void MovingTask_Edge_Rotate::sensorValuesChanged(SensorType sensorType){
	printf_s("TASK_ROTATE: \n");

	JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
	Variables_Bump bumpVar;
	Variables_Infrared infraredVar;
	{
		Sensor& sensor = ((MovingPlan_Base*)m_pPlanParent)->m_sensor;
		CAutoLock autoLock1(&(sensor.mLockBump));
		bumpVar = sensor.mBump;
		CAutoLock autoLock2(&(sensor.mLockInfrared));
		infraredVar = sensor.mInfrared;
	}

	int angle = 0;
	if (bumpVar.leftBump){
		//pRobot->setMoveType(MT_Distance, -100, 85);					// go backward 100mm at 85 mm/s(minimum speed)
		//addAction(MT_Distance, -100, 85);
		((JoyoungRobotImp*)pRobot)->setMoveType(MT_Distance, -300, 100, CMD_TYPE_BLOCK, 0);
		printf_s("TASK_ROTATE: Left bump! Go backward 100mm!\n");
		angle = 0;
	}
	if (bumpVar.rightBump){
		//pRobot->setMoveType(MT_Distance, -100, 85);					// go backward 100mm at 85 mm/s
		//addAction(MT_Distance, -100, 85);
		((JoyoungRobotImp*)pRobot)->setMoveType(MT_Distance, -300, 100, CMD_TYPE_BLOCK, 0);
		printf_s("TASK_ROTATE: Right bump! Go backward 100mm!\n");
		angle = 90;
	}
	if (infraredVar.infraredL1)
		angle = angle > 30 ? angle : 30;
	if (infraredVar.infraredL2)
		angle = angle > 60 ? angle : 60;
	if (infraredVar.infraredC)
		angle = 90;
	if (infraredVar.infraredR2)
		angle = 120;
	if (infraredVar.infraredR1)
		angle = 150;
	if (angle >= 30){
		//pRobot->setMoveType(MT_Angle, angle, 60);
		//addAction(MT_Angle, angle, 60);
		//((JoyoungRobotImp*)pRobot)->setMoveType(MT_Speed, 100, -100, CMD_TYPE_BLOCK, 16.82*angle+200);
		((JoyoungRobotImp*)pRobot)->setMoveType(MT_Angle, angle, 80, CMD_TYPE_BLOCK, 0);
		printf_s("TASK_ROTATE: Turn right %d degrees!\n", angle);
		return;
	}

	((JoyoungRobotImp*)pRobot)->setMoveType(MT_Stop, 0, 0, CMD_TYPE_BLOCK, 0);
	((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
	printf_s("TASK_ROTATE FINISHED! No bump and no infrared. \n");
}