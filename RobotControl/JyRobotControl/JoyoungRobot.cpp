#include "StdAfx.h"
#include "JoyoungRobot.h"
#include "Thread.h"

#include <stdio.h>

#include <string>
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

, m_openglThread(nullptr)
, encoderPathDrawer(&pathDrawer)

, m_moveType(MT_Stop)
, m_moveParam1(0)
, m_moveParam2(0)

, m_pReportProc(nullptr)
, m_pReportProcParam(nullptr)

, m_newReportData(false)

, m_robotMainThread(false)
{
	m_JYReportBuffer.clear();
	m_serialJyReadRows.clear();
	m_serialAdReadRows.clear();
}

JoyoungRobotImp::~JoyoungRobotImp()
{
    SafeDelete(m_movingPlanManager);

    SafeDelete(m_serialJyReadThread);

    SafeDelete(m_serialJyWriteThread);

    SafeDelete(m_serialAdReadThread);

	SafeDelete(m_openglThread);

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
		int nWriteBytes = serialJYPort_->write(setEncoderRateCommand, 9);									//set the encoder data update rate as 50Hz

        m_serialJyReadThread = new SerialThreadRead();
		if (m_serialJyReadThread && !m_serialJyReadThread->init(serialJYPort_, StatusReportPeriod_MinMS / 2, JoyoungRobotImp::processSerialJyReadBytes, this))
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

        m_serialAdReadThread = new SerialThreadRead();
		if (m_serialAdReadThread && !m_serialAdReadThread->init(serialADPort_, StatusReportPeriod_MinMS / 2, JoyoungRobotImp::processSerialAdReadBytes, this))
		{
			SafeDelete(m_serialAdReadThread);
			break;
		}
    }

	if (m_serialJyReadThread && (NULL == m_openglThread)){				//
		m_openglThread = new OpenglThread(this);
		m_robotMainThread = new RobotMainThread(this);
		m_movingPlanManager = new MovingPlanManagerImp(this);
	}
    return (m_serialJyReadThread || m_serialAdReadThread);
}

int	JoyoungRobotImp::setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2)
{
	if (NULL == m_serialJyReadThread)
		return -1;

	vecByte cmdBytes;
	{
		if(false ==m_jyProtocol.setMoveType2Bytes(moveType, moveParam1, moveParam2, cmdBytes))
			return -1;
	}

	m_serialJyWriteThread->pushWriteBytes(cmdBytes);

	{
		CAutoLock autoLock(&m_lockMoveType);
		m_moveType		=moveType;
		m_moveParam1	=moveParam1;
		m_moveParam2	=moveParam2;
	}
	return 0;
}
int	JoyoungRobotImp::setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2, CommandType commandType){
	switch (commandType){
	case CMD_TYPE_BLOCK:		//TODO: 该类型暂时无效，因为robotSensor的更新在本线程中，此处阻塞将导致程序死循环，可以将robotSensor的更新放在独立线程中避免这个问题
		//while (!m_serialJyWriteThread->isLastCommandSend()){
		//	Sleep(2);
		//}
		//if (m_moveType != MT_Speed){
		//	while (!robotSensor.isRobotStopped()){
		//		Sleep(2);
		//	}
		//}
		//setMoveType(moveType, moveParam1, moveParam2);
		//return 1;
	case CMD_TYPE_NONBLOCK:
		if (m_serialJyWriteThread->isLastCommandSend()){							//判断上一次命令是否发出
			if (m_moveType != MT_Speed){											//上一次命令对于除MT_Speed以外的其他模式，最终状态都是停止
				if (robotSensor.isRobotStopped()){									//通过判断停止来判断命令是否完成
					setMoveType(moveType, moveParam1, moveParam2);
					return 1;														//返回1表示命令被成功设置
				}
			}
			else{
				setMoveType(moveType, moveParam1, moveParam2);
				return 1;
			}
		}
		return 0;
	case CMD_TYPE_NOWAIT:
	default:
		setMoveType(moveType, moveParam1, moveParam2);
		return 1;
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
	CAutoLock autoLock(&m_lockRows);
	m_pReportProc		= pProc;
	m_pReportProcParam	= pProcParam;
	return false;
}

void JoyoungRobotImp::processSerialAdReadBytes(vecByte& reportBuffer, LPVOID pProcParam)
{
    JoyoungRobotImp* pThis = (JoyoungRobotImp*)pProcParam;
    pThis->processSerialAdReadBytes(reportBuffer);
}

void JoyoungRobotImp::processSerialAdReadBytes(vecByte& reportBuffer)
{
    CAutoLock autoLock(&m_lockRows);
    auto nOldRowSize = m_serialAdReadRows.size();
    m_jyProtocol.getRowsWithBytes(reportBuffer, m_serialAdReadRows);
    for (auto i = nOldRowSize; i < m_serialAdReadRows.size(); ++i)
    {
        const ArduinoRow& row = m_serialAdReadRows[i];
        switch (row.type)
        {
        case ArduinoRowType::ART_Ultrasonic:
            sensorVariablesChanged(m_pReportProcParam, SensorType::ST_Ultrasonic, 0, (LPVOID)&(row.param.ultrasonic), sizeof(row.param.ultrasonic));
            break;
        }
    }
}

void JoyoungRobotImp::processSerialJyReadBytes(vecByte& reportBuffer, LPVOID pProcParam)
{
    JoyoungRobotImp* pThis = (JoyoungRobotImp*)pProcParam;
    pThis->processSerialJyReadBytes(reportBuffer);
}

void JoyoungRobotImp::processSerialJyReadBytes(vecByte& reportBuffer)
{
	{
		CAutoLock autoLock(&m_lockReportBuffer);
		m_JYReportBuffer.insert(m_JYReportBuffer.end(), reportBuffer.begin(), reportBuffer.end());
		m_newReportData = true;
	}
	return;
}

void  JoyoungRobotImp::sensorVariablesChanged(LPVOID pProcParam,
    const SensorType sensorType, const int sensorIndex,
    const LPVOID sesorReportData, const int sesorReportSize)
{
    if (m_pReportProc)
        m_pReportProc(pProcParam, sensorType, sensorIndex, sesorReportData, sesorReportSize);
    MovingTask* curTask = m_movingPlanManager->taskCurrent();
	if (curTask)
		//((MovingTask_Base*)curTask)->envionmentVariables_Changed_Sensor(sensorType, sensorIndex, sesorReportData, sesorReportSize);
		((MovingTask_Base*)curTask)->sensorValuesChanged(sensorType);
}

void JoyoungRobotImp::mainProc(){
	size_t nOldRowSize;

	{
		CAutoLock autoLock(&m_lockReportBuffer);
		nOldRowSize = m_serialJyReadRows.size();
		if (m_JYReportBuffer.empty())
			return;
		m_jyProtocol.getRowsWithBytes(m_JYReportBuffer, m_serialJyReadRows);
	}
	//CAutoLock autoLock(&m_lockRows);
	for (auto i = nOldRowSize; i < m_serialJyReadRows.size(); ++i)
	{
		const JoyoungRow& row = m_serialJyReadRows[i];
		switch (row.type)
		{
		case JoyoungRowType::RRT_MotorEncoder:
			robotSensor.setSensorValues(ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
			sensorVariablesChanged(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
			if (m_openglThread){
				encoderDataChangedProc(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
			}
			break;
		case JoyoungRowType::RRT_Bump:
			robotSensor.setSensorValues(ST_Bump, 0, (LPVOID)&(row.param.bump), sizeof(row.param.bump));
			sensorVariablesChanged(m_pReportProcParam, SensorType::ST_Bump, 0, (LPVOID)&(row.param.bump), sizeof(row.param.bump));
			//printf_s("bump data: %d %d\n", robotSensor.mBump.leftBump, robotSensor.mBump.rightBump);
			break;
		case JoyoungRowType::RRT_Infrared:
			robotSensor.setSensorValues(ST_Infrared, 0, (LPVOID)&(row.param.infrared), sizeof(row.param.infrared));
			sensorVariablesChanged(m_pReportProcParam, SensorType::ST_Infrared, 0, (LPVOID)&(row.param.infrared), sizeof(row.param.infrared));
			printf_s("Infrared data: %d %d %d %d %d \n", robotSensor.mInfrared.infraredL1, robotSensor.mInfrared.infraredL2, \
				robotSensor.mInfrared.infraredC, robotSensor.mInfrared.infraredR2, robotSensor.mInfrared.infraredR1);
			break;
		case JoyoungRowType::RRT_WheelDrop:
			robotSensor.setSensorValues(ST_WheelDrop, 0, (LPVOID)&(row.param.drop), sizeof(row.param.drop));
			sensorVariablesChanged(m_pReportProcParam, SensorType::ST_WheelDrop, 0, (LPVOID)&(row.param.drop), sizeof(row.param.drop));
			break;
		default:
			break;
		}
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
