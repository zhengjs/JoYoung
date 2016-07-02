#include "stdafx.h"
#include "MovingTask_Edge_Along.h"
#include "MovingPlan_Base.h"

const static int    Speed_Default       = 120;
const static int    Differential_Max    = 12;

void MovingTask_Edge_Along::taskPlay()
{
    JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
    if (!pRobot)
        return;
    pRobot->setMoveType(MT_Speed, Speed_Default, Speed_Default);
	printf_s("current task: TASK_ALONG\n");
}

void MovingTask_Edge_Along::taskStop()
{
}

void MovingTask_Edge_Along::taskPause()
{
}

void MovingTask_Edge_Along::envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex,
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
    
    if (checkUnsafe2Stop(pRobot))
        return;

    do{
        int D2 = m_nUltrasonicVariables[2];
        int D3 = m_nUltrasonicVariables[3];

        if (0 == D2 || 0 == D3)
            break;

        int DefD    = 320;
        int MspD    = 240;
        int MspD2   = MspD / 2;
        int MinD    = DefD -MspD2;
        int MaxD    = DefD +MspD2;

        int ErrMinD = DefD - MspD;
        int ErrMaxD = DefD + MspD;

        if (ErrMaxD < D2 || ErrMaxD < D3)
        {
            printf("Task:(Along) Event:(%s)\n", "ErrMaxD < D2 || ErrMaxD < D3");
            break;//可能扫到空洞（门）
        }

        if (ErrMinD > D2/* || ErrMinD > D3*/)
        {
            pRobot->setMoveType(MT_Stop, 0, 0);
            ((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, (LPVOID)-1, 0);
            printf("Task:(Along) Event:(%s)\n", "ErrMinD > D2 || ErrMinD > D3");
            break;
        }

        if (D2 < MinD)
        {
            printf("Task:(Along) Event:(%s)\n", "D2 < MinD");
            auto fDScale = 1 - (D2 - ErrMinD) / (float)(MinD - ErrMinD);
            int nD = (int)(fDScale* Differential_Max);
            if (nD > 0)
            {
                pRobot->setMoveType(MT_Speed, Speed_Default + nD, Speed_Default - nD);
                printf("Task:(Along) Event:(L:%d R:%d)\n", nD, -nD);
            }
        }
        else if (D2 >MaxD)
        {
            printf("Task:(Along) Event:(%s)\n", "D2 >MaxD");
            auto fDScale = (ErrMaxD - D2) / (float)(ErrMaxD - MaxD);
            int nD = (int)(fDScale* Differential_Max);
            if (nD > 0)
            {
                pRobot->setMoveType(MT_Speed, Speed_Default - nD, Speed_Default + nD);
                printf("Task:(Along) Event:(L:%d R:%d)\n", -nD, nD);
            }
        }
        else
        {
            if (D2 > D3)
            {
                auto fDScale = min(1., (D2 - D3) / (float)MspD2);
                fDScale = sqrt(fDScale);
                int nD = (int)(fDScale* Differential_Max);
                if (nD > 0)
                {
                    pRobot->setMoveType(MT_Speed, Speed_Default - nD, Speed_Default+nD);
                    printf("Task:(Along) Event:(L:%d , R:%d)\n",nD, -nD);
                }
            }
            else
            {
                auto fDScale = min(1., (D3 - D2) / (float)MspD2);
                fDScale = sqrt(fDScale);
                int nD = (int)(fDScale* Differential_Max);
                if (nD > 0)
                {
                    pRobot->setMoveType(MT_Speed, Speed_Default+nD, Speed_Default - nD);
                    printf("Task:(Along) Event:(L:%d , R:%d)\n",nD, -nD);
                }
            }
        }
    }
    while (false);
}

bool MovingTask_Edge_Along::checkUnsafe2Stop(JoyoungRobot* pRobot)
{
    do 
    {
        if (0 == m_nUltrasonicVariables[0] || 0 == m_nUltrasonicVariables[1])
            break;
        if (m_nUltrasonicVariables[0] >= 240 && m_nUltrasonicVariables[1] >= 240)
            break;

        pRobot->setMoveType(MT_Stop, 0, 0);
        ((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
        return true;
    }
    while (false);

    return false;
}

void MovingTask_Edge_Along::sensorValuesChanged(SensorType sensorType){

	//if (sensorType != SensorType::ST_Infrared && sensorType != SensorType::ST_Bump)
	//	return;
	//printf_s("TASK_ALONG: DO COMMAND!\n");
	Sensor sensor = ((MovingPlan_Base*)m_pPlanParent)->m_sensor;
	JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
<<<<<<< HEAD
	if (sensor.mBump.rightBump || sensor.mInfrared.infraredC || sensor.mInfrared.infraredR2 || sensor.mInfrared.infraredR1){
=======
	if (sensor.mInfrared.infraredC || sensor.mInfrared.infraredR2 || sensor.mInfrared.infraredR1){
>>>>>>> origin/master
		pRobot->setMoveType(MT_Stop, 0, 0);
		printf_s("TASK_ALONG FINISHED! Infrared: %d %d %d %d %d\n", sensor.mInfrared.infraredL1, sensor.mInfrared.infraredL2, sensor.mInfrared.infraredC, sensor.mInfrared.infraredR2, sensor.mInfrared.infraredR1);
		((MovingPlan_Base*)m_pPlanParent)->taskFinished(this, nullptr, 0);
		return;
	}
<<<<<<< HEAD
	if (doCurrentAction())
		return;
	int speedL = 0, speedR = 0, time = 0;
	if (sensor.mBump.leftBump || sensor.mInfrared.infraredL1 || sensor.mInfrared.infraredL2){
		if (sensor.mBump.leftBump){
			speedL = -40;
			speedR = -80;
			time = 2000;
			pRobot->setMoveType(MT_Stop, 0, 0);									//先停下来
			addAction(MT_Speed, speedL , speedR, time);							// 2s
			printf_s("TASK_ALONG: Left bump! Set speed L%d R%d %dms\n", speedL, speedR, time);
			if (sensor.mInfrared.infraredL2){
				speedL = 80;
				speedR = 40;
				addAction(MT_Speed, speedL, speedR, time);
			}
=======
	if (sensor.mInfrared.infraredL1 || sensor.mInfrared.infraredL2){
		if (sensor.mBump.leftBump){
			pRobot->setMoveType(MT_Speed, -20, - 40);		//右轮速度降低
			printf_s("TASK_ALONG: Left bump! Set speed L%d R%d\n", -20,  - 40);
>>>>>>> origin/master
		}
		else if (sensor.mInfrared.infraredL2)
		{
			pRobot->setMoveType(MT_Speed, Speed_Default-30, Speed_Default - 70);
			printf_s("TASK_ALONG: Left2 infrared, no bump, set speed L%d R%d!\n", Speed_Default-30, Speed_Default - 70);
		}
		else{
			pRobot->setMoveType(MT_Speed, Speed_Default, Speed_Default);
			printf_s("TASK_ALONG: Left infrared, no bump, go forward!\n");
		}
<<<<<<< HEAD
		doCurrentAction();
=======
>>>>>>> origin/master
	}
	else{
		pRobot->setMoveType(MT_Speed, Speed_Default, Speed_Default + 5);				//右轮速度加快
		printf_s("TASK_ALONG: No infrared! Set speed L%d R%d \n", Speed_Default, Speed_Default + 5);
		printf_s("Infrared: %d %d %d %d %d\n", sensor.mInfrared.infraredL1, sensor.mInfrared.infraredL2, sensor.mInfrared.infraredC, sensor.mInfrared.infraredR2, sensor.mInfrared.infraredR1);
	}
	return;
}