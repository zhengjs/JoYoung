#pragma once

#include <vector>
#include "Global.h"
#include "Thread.h"
#include "SerialPort.h"
#include "JoyoungRobot.h"
#include "AutoLock.h"

class Sensor;
/******************************************************************************
SerialThreadRead
******************************************************************************/
typedef void(*processSerialReadBytesProc)(vecByte& reportBuffer, LPVOID pProcParam);

class SerialThreadRead : public ThreadProc_
{
public:
	SerialThreadRead(bool isJYPort)
		: m_pSerialPort_(nullptr)
		, m_nReadPeriod(StatusReportPeriod_MinMS / 2)
		, m_bJYPort(isJYPort)
	{}
	~SerialThreadRead(){
		SerialClose(m_pSerialPort_);
	}
	
	bool	init(SerialPort_* pSerialPort_, int nReadPeriod, Sensor* pSensor);		///*processSerialReadBytesProc pProcessProc, LPVOID pProcessProcParam*/
protected:
	void	Run(Thread_* pThread)override;
protected:
	SerialPort_*     	        m_pSerialPort_;

	int					        m_nReadPeriod;

	Thread_			            m_Thread_;
	Sensor*						m_pSensor;

	const bool					m_bJYPort;
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
	int getRowsWithBytes(vecByte& bytes, std::vector<rowType>& rows);
};

template<typename rowType>
int JoyoungRobotProtocol::getRowsWithBytes(vecByte& bytes, std::vector<rowType>& rows)
{
	LPBYTE pGetS = &bytes[0];
	LPBYTE pGetE = pGetS + bytes.size();
	LPBYTE pNextRow = pGetS;
	LPBYTE pReadRow = pGetS;

	int nRowsCount = rows.size();

	for (; pNextRow <pGetE;)
	{
		pReadRow = pNextRow;
		rowType row;
		bool bBufNoComplate = false;
		if (false == row.readHead(pReadRow, pGetE, pNextRow, bBufNoComplate))
		{
			if (TRUE == bBufNoComplate)
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

	if (pNextRow != pGetE)
		bytes.erase(bytes.begin(), bytes.begin() + (pNextRow - pGetS));
	else
		bytes.clear();

	return rows.size() - nRowsCount;
}

/******************************************************************************
SerialThreadWrite
******************************************************************************/

class SerialThreadWrite : public ThreadProc_
{
public:
	SerialThreadWrite(int nWritePeriod, SerialPort_* serialPort=nullptr)
		: ThreadProc_()
		, m_nWritePeriod(nWritePeriod)
		, m_serialPort(serialPort)
	{
		m_currCmd.cmdID = 0;
		m_currCmd.moveType = MT_NONE;
		m_currCmd.param1 = 0;
		m_currCmd.param2 = 0;
		m_currCmd.time = 0;
		m_currCmd.movetypeName = "";
		m_lastSendCmd.cmdID = 0;
		m_lastSendCmd.moveType = MT_NONE;
		m_lastSendCmd.param1 = 0;
		m_lastSendCmd.param2 = 0;
		m_lastSendCmd.time = 0;
		m_lastSendCmd.movetypeName = "";
		if (!m_Thread_.Start(this))
			;
	}
	~SerialThreadWrite(){
		SerialClose(m_serialPort);
	}
	void setSerialPort(SerialPort_* serialPort);
	bool isLastCommandSend(void);
	bool isLastCommandDone(DWORD time);
	int setCommand(const ControlCmd& currCmd);
protected:
	void	Run(Thread_* pThread)override;
	void	printSetMovetype(const ControlCmd& currCmd);
	void	pushWriteBytes(vecByte& writeBytes);
private:
	Thread_				m_Thread_;
	int					m_nWritePeriod;
	//size_t				m_lastVersion;
	//size_t				m_currVersion;
	SerialPort_*		m_serialPort;
	Lock_				m_writeBytesLock;
	vecByte				m_writeBytes;
	Lock_				m_currCmdLock, m_lastSendCmdLock;
	ControlCmd			m_currCmd,  m_lastSendCmd;
	JoyoungRobotProtocol m_JYProtocol;
};

