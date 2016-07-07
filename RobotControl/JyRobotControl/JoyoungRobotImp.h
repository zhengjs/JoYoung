#pragma once

#include <vector>
#include "JoyoungRobot.h"
#include "AutoLock.h"
#include "Global.h"
#include "sensor.h"
#include "encoderPathDrawer.h"
#include "serialThread.h"
#include "MovingPlan.h"


/******************************************************************************
JoyoungRobotImp
******************************************************************************/
class RobotMainThread;
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
	int    setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2, CommandType commandType, DWORD continueTime);
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
private:
	MovingPlanManagerImp*       m_movingPlanManager;

	SerialThreadRead*		    m_serialJyReadThread;

	SerialThreadWrite*          m_serialJyWriteThread;

	SerialThreadRead*		    m_serialAdReadThread;

	EncoderPathDrawer*			m_pathDrawerThread;

	Sensor*						m_pSensor;

	RobotMainThread*			m_robotMainThread;

	Lock_					    m_lockMoveType;
	ControlCmd					m_cmd;
	MoveType				    m_moveType;
	int						    m_moveParam1;
	int						    m_moveParam2;

	SensorVariablesChangedCallbackProc	m_pReportProc;
	LPVOID						m_pReportProcParam;
};


/*******************************************************************************
RobotMainThread
*******************************************************************************/

class RobotMainThread :public ThreadProc_{
public:
	RobotMainThread(JoyoungRobotImp* pRobot) :m_nPeriod(20), m_pRobot(pRobot){
		if (!m_Thread_.Start(this))
			return;
	}
protected:
	JoyoungRobotImp*			m_pRobot;
	void	Run(Thread_* pThread)override;
	int							m_nPeriod;
	Thread_			            m_Thread_;
};