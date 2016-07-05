#pragma once

#include <vector>
#include "JoyoungRobot.h"
#include "AutoLock.h"
#include "Global.h"

#include "Row_Joyoung.h"
#include "Row_Arduino.h"
#include "encoderPathDrawer.h"
#include "serialThread.h"
#include "MovingPlan.h"


typedef enum{
	CMD_TYPE_DEFAULT = 1,	//don't wait, may cover the last command
	CMD_TYPE_NOWAIT,		//
	CMD_TYPE_BLOCK,			//线程被阻塞直到释放（上一次动作完成）
	CMD_TYPE_NONBLOCK		//如果上一次动作没有完成，则函数立刻返回，本次命令被放弃
}CommandType;

class SerialPort_;

/*******************************************************************************
RobotMainThread
*******************************************************************************/

class RobotMainThread :public ThreadProc_{
public:
	RobotMainThread(JoyoungRobotImp* pRobot) :m_nPeriod(5),m_pRobot(pRobot){
		if (!m_Thread_.Start(this))
			return;
	}
protected:
	JoyoungRobotImp*			m_pRobot;
	void	Run(Thread_* pThread)override;
	int							m_nPeriod;
	Thread_			            m_Thread_;
};
/******************************************************************************
JoyoungRobotImp
******************************************************************************/

class JoyoungRobotImp : public JoyoungRobot
{
public:
	JoyoungRobotImp();
	virtual ~JoyoungRobotImp();

public:
	bool	init(const UINT& serialJyPort, const UINT& serialJyRate,
		const UINT& serialAdPort, const UINT& serialAdRate);

	MovingPlanManager* movingPlanManager()override{ return m_movingPlanManager; }
	int    setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2) override;
	int    setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2, CommandType commandType);
	void   getLastSetMoveType(MoveType& moveType, int& moveParam1, int& moveParam2) override;

	bool   setSensorVariablesChangedCallbackProc(SensorVariablesChangedCallbackProc pProc, LPVOID pProcParam,
		const int periodMS) override;

public:
	static void processSerialJyReadBytes(vecByte& readBytes, LPVOID pProcParam);
	void processSerialJyReadBytes(vecByte& readBytes);

	static void processSerialAdReadBytes(vecByte& readBytes, LPVOID pProcParam);
	void processSerialAdReadBytes(vecByte& readBytes);

	void  sensorVariablesChanged(LPVOID pProcParam,
		const SensorType sensorType, const int sensorIndex,
		const LPVOID sesorReportData, const int sesorReportSize);
	void mainProc(void);
public:
	MovingPlanManagerImp*       m_movingPlanManager;

	SerialThreadRead*		    m_serialJyReadThread;

	SerialThreadWrite*          m_serialJyWriteThread;

	SerialThreadRead*		    m_serialAdReadThread;

	OpenglThread*				m_openglThread;
	EncoderPathDrawer*			encoderPathDrawer;

	RobotMainThread*			m_robotMainThread;

	Lock_					    m_lockMoveType;
	MoveType				    m_moveType;
	int						    m_moveParam1;
	int						    m_moveParam2;

	Lock_					    m_lockRows;
	std::vector<JoyoungRow>		m_serialJyReadRows;
	std::vector<ArduinoRow>		m_serialAdReadRows;

	JoyoungRobotProtocol	    m_jyProtocol;

	SensorVariablesChangedCallbackProc	m_pReportProc;
	LPVOID						m_pReportProcParam;
private:

	vecByte						m_JYReportBuffer;
	Lock_						m_lockReportBuffer;
	bool						m_newReportData;
};
