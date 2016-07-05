#include "StdAfx.h"
#include "serialThread.h"
#include "AutoLock.h"
#include "Row_Arduino.h"
#include "Row_Joyoung.h"
#include "JoyoungRobotImp.h"
/******************************************************************************
SerialThreadRead
******************************************************************************/
bool SerialThreadRead::init(SerialPort_* pSerialPort_, int nReadPeriod, processSerialReadBytesProc pProc, LPVOID pProcParam)
{
	m_pSerialPort_ = pSerialPort_;
	m_nReadPeriod = nReadPeriod;
	m_pProcessProc = pProc;
	m_pProcessProcParam = pProcParam;

	if (!m_Thread_.Start(this))
		return false;

	return true;
}

void SerialThreadRead::Run(Thread_* pThread)
{
	int		nRead;
	BYTE	readBuffer[1000];
	vecByte	reportBuffer;
	do
	{
		if (pThread->wantStop(m_nReadPeriod))
			break;

		nRead = m_pSerialPort_->read(readBuffer, 1000);
		if (0 == nRead)
			continue;

		reportBuffer.resize(nRead);
		memcpy(&reportBuffer[0], readBuffer, nRead);

		if (m_pProcessProc){
			m_pProcessProc(reportBuffer, m_pProcessProcParam);
			reportBuffer.clear();
			nRead = 0;
		}
	} while (true);
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
			CAutoLock autoLock(&m_writeBytesLock);
			if (m_currVersion>m_lastVersion && m_serialPort){					// new data and valid serial port
				int nWriteBytes = m_serialPort->write(&m_writeBytes[0], m_writeBytes.size());
				m_lastVersion = m_currVersion;
			}
		}
	} while (true);
}

void SerialThreadWrite::setSerialPort(SerialPort_* serialPort){
	if (serialPort){
		if (m_serialPort != nullptr && m_serialPort != serialPort){
			SerialClose(m_serialPort);
		}
		m_serialPort = serialPort;
	}
	else
		return;
}

void SerialThreadWrite::pushWriteBytes(vecByte& writeBytes){
	{
		CAutoLock autoLock(&m_writeBytesLock);
		m_writeBytes = writeBytes;
		m_currVersion++;
	}
}
bool SerialThreadWrite::isLastCommandSend(void){
	return (m_lastVersion == m_currVersion);
}
int  SerialThreadWrite::numOfLostCommand(){
	return (m_currVersion - m_lastVersion);
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
	INT16 nParam1 = moveParam1;
	INT16 nParam2 = moveParam2;

	switch (moveType)
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
						 0x5a };

					 WORD* mtL = (WORD*)(buf + 5);
					 WORD* mtR = mtL + 1;
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
