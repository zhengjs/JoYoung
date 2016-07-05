#pragma once

#include "Global.h"
#include <vector>
#include "Thread.h"
#include "SerialPort.h"
#include "JoyoungRobot.h"
#include "AutoLock.h"


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
	~SerialThreadRead(){
		SerialClose(m_pSerialPort_);
	}

	bool	init(SerialPort_* pSerialPort_, int nReadPeriod, processSerialReadBytesProc pProcessProc, LPVOID pProcessProcParam);
protected:
	void	Run(Thread_* pThread)override;
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
//class JoyoungRobotImp;

class SerialThreadWrite : public ThreadProc_
{
public:
	SerialThreadWrite(int nWritePeriod, SerialPort_* serialPort=nullptr)
		: ThreadProc_()
		, m_nWritePeriod(nWritePeriod)
		, m_lastVersion(0), m_currVersion(0)
		, m_serialPort(serialPort)
	{
		if (!m_Thread_.Start(this))
			;
	}
	~SerialThreadWrite(){
		SerialClose(m_serialPort);
	}
	void setSerialPort(SerialPort_* serialPort);
	void pushWriteBytes(vecByte& writeBytes);
	bool isLastCommandSend(void);
	int  numOfLostCommand();
protected:
	void	Run(Thread_* pThread)override;

protected:
	Thread_				m_Thread_;
	Lock_				m_writeBytesLock;
	int					m_nWritePeriod;
	size_t				m_lastVersion;
	size_t				m_currVersion;
	SerialPort_*		m_serialPort;
	vecByte				m_writeBytes;
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

