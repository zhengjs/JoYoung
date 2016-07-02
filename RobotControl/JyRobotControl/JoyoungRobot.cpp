#include "StdAfx.h"
#include "JoyoungRobot.h"
#include "Thread.h"
#include "SerialPort.h"
#include "AutoLock.h"

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

#include "Row_Joyoung.h"
#include "Row_Arduino.h"

#include "MovingPlan.h"
#include "MovingTask_Base.h"

#include "encoderPathDrawer.h"
#include "GL/freeglut.h"
#include "sensor.h"

#include <atomic>

/******************************************************************************
SerialThreadRead
******************************************************************************/
typedef void(*processSerialReadBytesProc)(vecByte& reportBuffer, LPVOID pProcParam);

class SerialThreadRead : public ThreadProc_
{
public:
    SerialThreadRead()
        : m_pSerialPort_(nullptr)
        , m_nReadPeriod(StatusReportPeriod_MinMS / 2)
        , m_pProcessProc(nullptr)
        , m_pProcessProcParam(NULL)
    {}

    bool	init(SerialPort_* pSerialPort_, int nReadPeriod, processSerialReadBytesProc pProcessProc, LPVOID pProcessProcParam);
protected:
	void	Run	(Thread_* pThread)override;
protected:
    SerialPort_*     	        m_pSerialPort_;

    int					        m_nReadPeriod;
	
    processSerialReadBytesProc  m_pProcessProc;
    LPVOID                      m_pProcessProcParam;

    Thread_			            m_Thread_;
};

/******************************************************************************
SerialThreadWrite
******************************************************************************/
class JoyoungRobotImp;

class SerialThreadWrite : public ThreadProc_
{
public:
    SerialThreadWrite(JoyoungRobotImp* pRobot, int nWritePeriod) 
        : ThreadProc_()
        , m_pRobot(pRobot)
        , m_nWritePeriod(nWritePeriod)
        , m_lastWriteBytesVersion(0)
    {
        if (!m_Thread_.Start(this))
            ;
    }

protected:
    void	Run(Thread_* pThread)override;

protected:
    Thread_  m_Thread_;

    int     m_nWritePeriod;
    int     m_lastWriteBytesVersion;
    
    JoyoungRobotImp* m_pRobot;
};

/****************************************************************************
openGl thread
	类Thread_封装了创建线程及执行线程函数的一系列操作，当需要创建一个线程执行特定操作时，
	只需要创建该操作的类，并实现ThreadProc_接口，复写Run方法，定义一个Thread_成员，最后
	在合适的时刻调用Thread_成员的Start方法，线程即启动
******************************************************************************/
class OpenglThread :public ThreadProc_
{
public:
	OpenglThread(JoyoungRobotImp* pRobot) :ThreadProc_(), m_pRobot(pRobot)
	{
		if (!m_Thread_.Start(this))
			return;
	}
protected:
	void Run(Thread_* pThread) override{
		initPathDrawer();
		do{
			glutPostRedisplay();
			//printf_s("opengl thread runing!\n");
			glutMainLoopEvent();
			Sleep(5);
		} while (true);
	}
private:
	Thread_ m_Thread_;
	JoyoungRobotImp* m_pRobot;
};
/******************************************************************************
JoyoungRobotProtocol
******************************************************************************/

class JoyoungRobotProtocol
{
public:
	JoyoungRobotProtocol();
	~JoyoungRobotProtocol();
	
	bool setMoveType2Bytes(const MoveType& moveType, const int& moveParam1, const int& moveParam2, 
								  vecByte& bytes);
    template<typename rowType>
    int getRowsWithBytes(vecByte& bytes, vector<rowType>& rows);
};

/******************************************************************************
JoyoungRobotImp
******************************************************************************/
class MovingPlanManagerImp :public MovingPlanManager
{
public:
    MovingPlanManagerImp(JoyoungRobot* pRobot) :MovingPlanManager(pRobot){}
};

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

public:
    MovingPlanManagerImp*       m_movingPlanManager;

	SerialPort_*				m_serialJyPort;
	SerialThreadRead*		    m_serialJyReadThread;

    Lock_                       m_lockWriteInfo;
    vecByte                     m_lastWriteBytes;
    int                         m_lastWriteBytesVersion;
    SerialThreadWrite*          m_serialJyWriteThread;

    SerialPort_*			    m_serialAdPort;
    SerialThreadRead*		    m_serialAdReadThread;

	OpenglThread*				m_openglThread;
	EncoderPathDrawer*			encoderPathDrawer;

	Lock_					    m_lockMoveType;
	MoveType				    m_moveType;
	int						    m_moveParam1;
	int						    m_moveParam2;

	Lock_					    m_lockRows;
    vector<JoyoungRow>		    m_serialJyReadRows;
    vector<ArduinoRow>		    m_serialAdReadRows;

	JoyoungRobotProtocol	    m_jyProtocol;

	SensorVariablesChangedCallbackProc	m_pReportProc;
	LPVOID						m_pReportProcParam;
};

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
, m_serialJyPort(nullptr)
, m_serialJyReadThread(nullptr)

, m_serialJyWriteThread(nullptr)
, m_lastWriteBytesVersion(0)

, m_serialAdPort(nullptr)
, m_serialAdReadThread(nullptr)

, m_openglThread(nullptr)
, encoderPathDrawer(&pathDrawer)

, m_moveType(MT_Stop)
, m_moveParam1(0)
, m_moveParam2(0)

, m_pReportProc(nullptr)
, m_pReportProcParam(nullptr)
{
}

JoyoungRobotImp::~JoyoungRobotImp()
{
    SafeDelete(m_movingPlanManager);

    SafeDelete(m_serialJyReadThread);
	SerialClose(m_serialJyPort);

    SafeDelete(m_serialJyWriteThread);

    SafeDelete(m_serialAdReadThread);
    SerialClose(m_serialAdPort);

	SafeDelete(m_openglThread);
}

bool JoyoungRobotImp::init(const UINT& serialJyPort, const UINT& serialJyRate,
                           const UINT& serialAdPort, const UINT& serialAdRate)
{
    while (NULL == m_serialJyReadThread)
    {
        if (0 == serialJyPort)
            break;

        SafeDelete(m_serialJyPort);
        if (NULL == (m_serialJyPort = SerialOpen(serialJyPort, serialJyRate)))
            break;

		BYTE setEncoderRateCommand[9] = {0xA5, 0x00, 0x06, 0x10, 0x00, 0x00, 0x14, 0x00, 0x5a};
		setEncoderRateCommand[7] = getBytes_Xor(setEncoderRateCommand + 1, setEncoderRateCommand+7);
		int nWriteBytes = m_serialJyPort->write(setEncoderRateCommand, 9);									//set the encoder data update rate as 50Hz

        m_serialJyReadThread = new SerialThreadRead();
        if (m_serialJyReadThread &&!m_serialJyReadThread->init(m_serialJyPort, StatusReportPeriod_MinMS / 2, JoyoungRobotImp::processSerialJyReadBytes, this))
        {
            SafeDelete(m_serialJyReadThread);
            break;
        }

        m_serialJyWriteThread = new SerialThreadWrite(this, 100);
    }

    while (NULL == m_serialAdReadThread)
    {
        if (0 == serialAdPort)
            break;

        SafeDelete(m_serialAdPort)
        if (NULL == (m_serialAdPort = SerialOpen(serialAdPort, serialAdRate)))
            break;

        m_serialAdReadThread = new SerialThreadRead();
        if (m_serialAdReadThread &&!m_serialAdReadThread->init(m_serialAdPort, StatusReportPeriod_MinMS / 2, JoyoungRobotImp::processSerialAdReadBytes, this))
            return false;

    }

	if (m_serialJyReadThread && (NULL == m_openglThread)){				//
		m_openglThread = new OpenglThread(this);
	}
    m_movingPlanManager = new MovingPlanManagerImp(this);
    return (m_serialJyReadThread || m_serialAdReadThread);
}

int	JoyoungRobotImp::setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2)
{
	if (NULL == m_serialJyPort)
		return -1;

	vecByte cmdBytes;
	{
		if(false ==m_jyProtocol.setMoveType2Bytes(moveType, moveParam1, moveParam2, cmdBytes))
			return -1;
	}

	{
		CAutoLock autoLock(&m_lockWriteInfo);
        m_lastWriteBytes = cmdBytes;
        ++m_lastWriteBytesVersion;
	}
	{
		CAutoLock autoLock(&m_lockMoveType);
		m_moveType		=moveType;
		m_moveParam1	=moveParam1;
		m_moveParam2	=moveParam2;
	}
	return 0;
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
	CAutoLock autoLock(&m_lockRows);
	auto nOldRowSize = m_serialJyReadRows.size();
	m_jyProtocol.getRowsWithBytes(reportBuffer, m_serialJyReadRows);
    for (auto i = nOldRowSize; i < m_serialJyReadRows.size(); ++i)
    {
        const JoyoungRow& row = m_serialJyReadRows[i];
        switch (row.type)
        {
        case JoyoungRowType::RRT_MotorEncoder:
            sensorVariablesChanged(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
			if (m_openglThread){
				encoderDataChangedProc(m_pReportProcParam, SensorType::ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
			}
			robotSensor.setSensorValues(ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
            break;
        case JoyoungRowType::RRT_Bump:
            sensorVariablesChanged(m_pReportProcParam, SensorType::ST_Bump, 0, (LPVOID)&(row.param.bump), sizeof(row.param.bump));
			robotSensor.setSensorValues(ST_Bump, 0, (LPVOID)&(row.param.bump), sizeof(row.param.bump));
			//printf_s("bump data: %d %d\n", robotSensor.mBump.leftBump, robotSensor.mBump.rightBump);
            break;
        case JoyoungRowType::RRT_Infrared:
            sensorVariablesChanged(m_pReportProcParam, SensorType::ST_Infrared, 0, (LPVOID)&(row.param.infrared), sizeof(row.param.infrared));
			robotSensor.setSensorValues(ST_Infrared, 0, (LPVOID)&(row.param.infrared), sizeof(row.param.infrared));
			printf_s("Infrared data: %d %d %d %d %d \n", robotSensor.mInfrared.infraredL1, robotSensor.mInfrared.infraredL2, \
					robotSensor.mInfrared.infraredC, robotSensor.mInfrared.infraredR2, robotSensor.mInfrared.infraredR1 );
            break;
        case JoyoungRowType::RRT_WheelDrop:
            sensorVariablesChanged(m_pReportProcParam, SensorType::ST_WheelDrop, 0, (LPVOID)&(row.param.drop), sizeof(row.param.drop));
			robotSensor.setSensorValues(ST_WheelDrop, 0, (LPVOID)&(row.param.drop), sizeof(row.param.drop));
            break;
        default:
            break;
        }
    }
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

/******************************************************************************
SerialThreadRead
******************************************************************************/
bool SerialThreadRead::init(SerialPort_* pSerialPort_, int nReadPeriod, processSerialReadBytesProc pProc, LPVOID pProcParam)
{
    m_pSerialPort_       = pSerialPort_;
    m_nReadPeriod       = nReadPeriod;
    m_pProcessProc      = pProc;
    m_pProcessProcParam = pProcParam;

	if(!m_Thread_.Start(this))
		return false;

    return true;
}

void SerialThreadRead::Run(Thread_* pThread)
{
	int		waitStopState =0, nRead, nOldBufSize;
	BYTE	readBuffer[1000];
	vecByte	reportBuffer;
	do 
	{
		if ( pThread->wantStop(m_nReadPeriod) )
			break;

        nRead = m_pSerialPort_->read(readBuffer, 1000);
		if(0 ==nRead)
			continue;

		nOldBufSize =reportBuffer.size();
		reportBuffer.resize(nOldBufSize +nRead);
		memcpy(&reportBuffer[0] +nOldBufSize, readBuffer, nRead);
        
        if (m_pProcessProc)
            m_pProcessProc(reportBuffer, m_pProcessProcParam);
	} 
	while (true);
}
/******************************************************************************
SerialThreadWrite
******************************************************************************/
void SerialThreadWrite::Run(Thread_* pThread)
{
    do
    {
        if (pThread->wantStop(m_nWritePeriod))
            break;
        {
            CAutoLock autoLock(&m_pRobot->m_lockWriteInfo);
            if (m_lastWriteBytesVersion != m_pRobot->m_lastWriteBytesVersion)
            {
                int nWriteBytes = m_pRobot->m_serialJyPort->write(&m_pRobot->m_lastWriteBytes[0], m_pRobot->m_lastWriteBytes.size());
                m_lastWriteBytesVersion = m_pRobot->m_lastWriteBytesVersion;
            }
        }
    } while (true);
}
/******************************************************************************
JoyoungRobotProtocol
******************************************************************************/
JoyoungRobotProtocol::JoyoungRobotProtocol()
{
};

JoyoungRobotProtocol::~JoyoungRobotProtocol()
{
}


bool JoyoungRobotProtocol::setMoveType2Bytes(const MoveType& moveType, const int& moveParam1, const int& moveParam2, vecByte& bytes)
{
    INT16 nParam1   = moveParam1;
	INT16 nParam2   = moveParam2;

	switch(moveType)
	{
	case MT_Stop:
        nParam1 = 0;
        nParam2 = 0;
	case MT_Speed:
		{
            const int btSize = 11;
            bytes.resize(btSize);
            BYTE buf[btSize] = { 0xA5,
				0x00, 0x08, 
				0x00, 
				0x70, 
				0x00, 0x00, 
				0x00, 0x00, 
				0x00, 
				0x5a};

			WORD* mtL		=(WORD*)(buf +5);
			WORD* mtR		=mtL +1;
            *mtL = ((nParam1 & 0xFF) << 8) + ((nParam1 & 0xFF00) >> 8);
            *mtR = ((nParam2 & 0xFF) << 8) + ((nParam2 & 0xFF00) >> 8);

            *(BYTE*)(mtR + 1) = getBytes_Xor(buf + 1, buf + btSize - 2);
			memcpy(&bytes[0], buf, bytes.size());
		}
		return true;
	case MT_Distance:
		{
			const int btSize = 12;
			bytes.resize(btSize);
			BYTE buf[btSize] = { 0xA5,
				0x00, 0x09,
				0x00,
				0x72,
				0x00, 0x00,
				0x00,
				0x00, 0x00,
				0x00,
				0x5A };
			WORD* mileage = (WORD*)(buf + 5);
			WORD* speed = (WORD*)(buf + 8);
			buf[7] = nParam1 > 0 ? 1 : 2;
			nParam1 = abs(nParam1);
			*mileage = ((nParam1 & 0xFF) << 8) + ((nParam1 & 0xFF00) >> 8);
			*speed = ((nParam2 & 0xFF) << 8) + ((nParam2 & 0xFF00) >> 8);;
			buf[10] = getBytes_Xor(buf + 1, buf + btSize - 2);
			memcpy(&bytes[0], buf, bytes.size());
		}
		return true;
    case MT_Angle:
        {
            const int btSize = 12;
            bytes.resize(btSize);
            BYTE buf[btSize] = { 0xA5,
                0x00, 0x09,
                0x00,
                0x73,
                0x00, 0x00,
                0x00,
                0x00, 0x00,
                0x00,
                0x5a };

            WORD* angT = (WORD*)(buf + 5);
            BYTE* angO = (BYTE*)(buf + 7);
            WORD* angS = (WORD*)(buf + 8);
            *angO = nParam1 >0 ? 2 : 1;
            nParam1 = abs(nParam1);
            *angT = ((nParam1 & 0xFF) << 8) + ((nParam1 & 0xFF00) >> 8);
            *angS = ((nParam2 & 0xFF) << 8) + ((nParam2 & 0xFF00) >> 8);

            *(BYTE*)(angS + 1) = getBytes_Xor(buf + 1, buf + btSize - 2);
            memcpy(&bytes[0], buf, bytes.size());
        }
        return true;
	}
	return false;
};

template<typename rowType>
int JoyoungRobotProtocol::getRowsWithBytes(vecByte& bytes, vector<rowType>& rows)
{
	LPBYTE pGetS	=&bytes[0];
	LPBYTE pGetE	=pGetS +bytes.size();
	LPBYTE pNextRow	=pGetS;
	LPBYTE pReadRow	=pGetS;

	int nRowsCount	=rows.size();

	for(; pNextRow <pGetE ;)
	{
        pReadRow = pNextRow;
        rowType row;
		bool bBufNoComplate	=false;
        if (false == row.readHead(pReadRow, pGetE, pNextRow, bBufNoComplate))
		{
			if(TRUE == bBufNoComplate)
			{
                pNextRow = pReadRow;
				break;
			}
			else
				continue;
		}

        if (false == row.readType(pReadRow, pGetE))
			continue;

        row.readIndex(pReadRow, pGetE);
        row.readParam(pReadRow, pGetE);

		rows.push_back(row);
	}
	
	if(pNextRow !=pGetE)
		bytes.erase(bytes.begin(), bytes.begin() +(pNextRow -pGetS));
	else
		bytes.clear();

	return rows.size()-nRowsCount;
}
