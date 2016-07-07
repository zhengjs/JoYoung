#include "StdAfx.h"
#include "JoyoungRobot.h"
#include "Thread.h"

#include <stdio.h>

#include <map>
#include <list>
#include <vector>
using namespace std;

#define _USE_MATH_DEFINES
#include <math.h>
#include <tchar.h>
#include "Global.h"

#include "MovingTask_Base.h"
#include "JoyoungRobotImp.h"

#include "sensor.h"

#include <atomic>


/******************************************************************************
JoyoungRobot::getSingleton
******************************************************************************/
static JoyoungRobotImp* imp =nullptr;

JoyoungRobot* JoyoungRobot::connectRobot(const UINT& serialJyPort, const UINT& serialJyRate, const UINT& serialAdPort, const UINT& serialAdRate)
{
    if (!imp)
    {
        imp = new JoyoungRobotImp();
        if (!imp->init(serialJyPort, serialJyRate, serialAdPort, serialAdRate))
        {
            SafeDelete(imp);
        };
    }
	return imp;
}

void JoyoungRobot::disconnectRobot(JoyoungRobot* robot)
{
    SafeDelete(imp);
}

/******************************************************************************
JoyoungRobotImp
******************************************************************************/

JoyoungRobotImp::JoyoungRobotImp()
: m_movingPlanManager(nullptr)
, m_serialJyReadThread(nullptr)

, m_serialJyWriteThread(nullptr)

, m_serialAdReadThread(nullptr)

, m_pathDrawerThread(&pathDrawer)
, m_pSensor(&robotSensor)

, m_moveType(MT_Stop)
, m_moveParam1(0)
, m_moveParam2(0)

, m_pReportProc(nullptr)
, m_pReportProcParam(nullptr)


, m_robotMainThread(false)
{
	m_cmd.cmdID = 0;
	m_cmd.cmdType = CMD_TYPE_DEFAULT;
	m_cmd.continueTime = 0;
}

JoyoungRobotImp::~JoyoungRobotImp()
{
    SafeDelete(m_movingPlanManager);

    SafeDelete(m_serialJyReadThread);

    SafeDelete(m_serialJyWriteThread);

    SafeDelete(m_serialAdReadThread);

	SafeDelete(m_robotMainThread);
}

bool JoyoungRobotImp::init(const UINT& serialJyPort, const UINT& serialJyRate,
                           const UINT& serialAdPort, const UINT& serialAdRate)
{
    while (NULL == m_serialJyReadThread)
    {
        if (0 == serialJyPort)
            break;

		SerialPort_* serialJYPort_;
		if (NULL == (serialJYPort_ = SerialOpen(serialJyPort, serialJyRate)))
            break;

		BYTE setEncoderRateCommand[9] = {0xA5, 0x00, 0x06, 0x10, 0x00, 0x00, 0x14, 0x00, 0x5a};
		setEncoderRateCommand[7] = getBytes_Xor(setEncoderRateCommand + 1, setEncoderRateCommand+7);
		int nWriteBytes = serialJYPort_->write(setEncoderRateCommand, 9);							//set the encoder data update rate as 50Hz

        m_serialJyReadThread = new SerialThreadRead(true);
		if (m_serialJyReadThread && !m_serialJyReadThread->init(serialJYPort_, StatusReportPeriod_MinMS / 2, &robotSensor))
        {
            SafeDelete(m_serialJyReadThread);
            break;
        }

		m_serialJyWriteThread = new SerialThreadWrite(10, serialJYPort_);
    }

    while (NULL == m_serialAdReadThread)
    {
        if (0 == serialAdPort)
            break;

		SerialPort_* serialADPort_;
		if (NULL == (serialADPort_ = SerialOpen(serialAdPort, serialAdRate)))
            break;

        m_serialAdReadThread = new SerialThreadRead(false);
		if (m_serialAdReadThread && !m_serialAdReadThread->init(serialADPort_, StatusReportPeriod_MinMS / 2, &robotSensor))
		{
			SafeDelete(m_serialAdReadThread);
			break;
		}
    }

	if (m_serialJyReadThread){										//
		m_pSensor->init();											//start sensor thread
		m_pathDrawerThread->init();									//start opengl thread
		if (NULL == m_robotMainThread)
			m_robotMainThread = new RobotMainThread(this);
		if (NULL == m_movingPlanManager)
			m_movingPlanManager = new MovingPlanManagerImp(this);
	}
    return (m_serialJyReadThread || m_serialAdReadThread);
}

int	JoyoungRobotImp::setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2)
{
	if (NULL == m_serialJyReadThread)
		return -1;

	std::string movetypeName;
	switch (moveType)
	{
	case MT_Speed:
		movetypeName = "MT_Speed";
		break;

	case MT_Stop:
		movetypeName = "MT_Stop";
		break;

	case MT_Angle:
		movetypeName = "MT_Angle";
		break;

	case MT_Distance:
		movetypeName = "MT_Distance";
		break;
	default:
		movetypeName = "MT_None";
		break;
	}
	m_cmd.cmdID++;
	m_cmd.moveType = moveType;
	m_cmd.param1 = moveParam1;
	m_cmd.param2 = moveParam2;
	m_cmd.movetypeName = movetypeName;
	{
		CAutoLock autoLock(&(m_pSensor->mLockEncoder));
		m_cmd.time = m_pSensor->mEncoder.stamp;
	}
	m_serialJyWriteThread->setCommand(m_cmd);
	m_cmd.cmdType = CMD_TYPE_DEFAULT;
	m_cmd.continueTime = 0;

	{
		CAutoLock autoLock(&m_lockMoveType);
		m_moveType		=moveType;
		m_moveParam1	=moveParam1;
		m_moveParam2	=moveParam2;
	}
	return 0;
}
int	JoyoungRobotImp::setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2, CommandType commandType, DWORD continueTime){

	m_cmd.cmdType = commandType;
	m_cmd.continueTime = continueTime;
	DWORD time;
	switch (commandType)
	{
	case CMD_TYPE_BLOCK:
		do{
			{
				CAutoLock autoLock(&(m_pSensor->mLockEncoder));
				time = m_pSensor->mEncoder.stamp;
			}
			Sleep(5);
		} while (!m_serialJyWriteThread->isLastCommandDone(time));
		setMoveType(moveType, moveParam1, moveParam2);
		return 0;
	case CMD_TYPE_NONBLOCK:
		{
			CAutoLock autoLock(&(m_pSensor->mLockEncoder));
			time = m_pSensor->mEncoder.stamp;
		}
		if (!m_serialJyWriteThread->isLastCommandDone(time))
			return -1;
		else{
			setMoveType(moveType, moveParam1, moveParam2);
			return 0;
		}
	case CMD_TYPE_NOWAIT:
	default:
		setMoveType(moveType, moveParam1, moveParam2);
		return 0;
	}
}

void JoyoungRobotImp::getLastSetMoveType(MoveType& moveType, int& moveParam1, int& moveParam2)
{
	CAutoLock autoLock(&m_lockMoveType);
	moveType	=m_moveType;
	moveParam1	=m_moveParam1;
	moveParam2	=m_moveParam2;
}

bool JoyoungRobotImp::setSensorVariablesChangedCallbackProc(SensorVariablesChangedCallbackProc pProc, LPVOID pProcParam, const int periodMS)
{
	m_pReportProc		= pProc;
	m_pReportProcParam	= pProcParam;
	return false;
}

void  JoyoungRobotImp::sensorVariablesChanged(LPVOID pProcParam,
    const SensorType sensorType, const int sensorIndex,
    const LPVOID sesorReportData, const int sesorReportSize)
{
    if (m_pReportProc)
        m_pReportProc(pProcParam, sensorType, sensorIndex, sesorReportData, sesorReportSize);
    MovingTask* curTask = m_movingPlanManager->taskCurrent();
	if (curTask)
		((MovingTask_Base*)curTask)->sensorValuesChanged(sensorType);
}

void JoyoungRobotImp::mainProc(){	
	if (m_pSensor->mbNewEncoder){
		Variables_MotorEncoder encoderVar;
		{
			CAutoLock autoLock(&(m_pSensor->mLockEncoder));
			encoderVar = m_pSensor->mEncoder;
			m_pSensor->mbNewEncoder = false;
		}
		sensorVariablesChanged(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(encoderVar), sizeof(encoderVar));
	}
	if (m_pSensor->mbNewBump){
		Variables_Bump bumpVar;
		{
			CAutoLock autoLock(&(m_pSensor->mLockBump));
			bumpVar = m_pSensor->mBump;
			m_pSensor->mbNewBump = false;
		}
		sensorVariablesChanged(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(bumpVar), sizeof(bumpVar));
	}
	if (m_pSensor->mbNewInfrared){
		Variables_Infrared infraredVar;
		{
			CAutoLock autoLock(&(m_pSensor->mLockInfrared));
			infraredVar = m_pSensor->mInfrared;
			m_pSensor->mbNewInfrared = false;
		}
		sensorVariablesChanged(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(infraredVar), sizeof(infraredVar));
	}
	if (m_pSensor->mbNewWheelDrop){
		Variables_WheelDrop wheeldropVar;
		{
			CAutoLock autoLock(&(m_pSensor->mLockWheelDrop));
			wheeldropVar = m_pSensor->mWheelDrop;
			m_pSensor->mbNewWheelDrop = false;
		}
		sensorVariablesChanged(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(wheeldropVar), sizeof(wheeldropVar));
	}
}

void RobotMainThread::Run(Thread_* pThread){
	do{
		if (pThread->wantStop(m_nPeriod))
			break;
		if (m_pRobot){
			m_pRobot->mainProc();
		}
	} while (true);
}


BYTE getBytes_Xor(const LPBYTE pStart, const LPBYTE pEnd)
{
	BYTE ret = 0;
	for (LPBYTE pByte = pStart; pByte < pEnd; ++pByte)
		ret ^= *pByte;
	return ret;
}
