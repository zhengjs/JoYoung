#include "StdAfx.h"
#include "serialThread.h"
#include "sensor.h"

/******************************************************************************
SerialThreadRead
******************************************************************************/
bool SerialThreadRead::init(SerialPort_* pSerialPort_, int nReadPeriod, Sensor* pSensor)		///*, processSerialReadBytesProc pProc, LPVOID pProcParam*/
{
	m_pSerialPort_ = pSerialPort_;
	m_nReadPeriod = nReadPeriod;
	m_pSensor = pSensor;

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

		if (m_pSensor){
			if (m_bJYPort){
				m_pSensor->processSerialJyReadBytes(reportBuffer);
				//printf_s("JY serial read thread running!\n");
			}
			else{
				m_pSensor->processSerialAdReadBytes(reportBuffer);
				//printf_s("AD serial read thread running!\n");
			}
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
			CAutoLock autoLock1(&m_writeBytesLock);
			CAutoLock autoLock2(&m_currCmdLock);
			CAutoLock autoLock3(&m_lastSendCmdLock);
			if (m_currCmd.cmdID>m_lastSendCmd.cmdID && m_serialPort){					// new cmd and valid serial port
				int nWriteBytes = m_serialPort->write(&m_writeBytes[0], m_writeBytes.size());
				//printf_s("JY serial write thread running!!!\n");
				printf_s("Send movetype %s, param1=%d, param2=%d\n", m_currCmd.movetypeName.c_str(), m_currCmd.param1, m_currCmd.param2);
				m_lastSendCmd = m_currCmd;
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

int SerialThreadWrite::setCommand(const ControlCmd& currCmd){
	{
		CAutoLock autoLock(&m_currCmdLock);
		m_currCmd = currCmd;
	}

	vecByte cmdBytes;
	{
		if (false == m_JYProtocol.setMoveType2Bytes(currCmd.moveType, currCmd.param1, currCmd.param2, cmdBytes))
			return -1;
	}
	{
		CAutoLock autoLock(&m_writeBytesLock);
		m_writeBytes = cmdBytes;
	}
	printSetMovetype(currCmd);
}
bool SerialThreadWrite::isLastCommandSend(void){
	{
		CAutoLock autoLock1(&m_currCmdLock);
		CAutoLock autoLock2(&m_lastSendCmdLock);
		return (m_currCmd.cmdID == m_lastSendCmd.cmdID);
	}
}
bool SerialThreadWrite::isLastCommandDone(DWORD time){

	if (!isLastCommandSend())
		return false;
	{
		CAutoLock autoLock1(&m_currCmdLock);
		CAutoLock autoLock2(&m_lastSendCmdLock);

		if (m_lastSendCmd.moveType != MT_Speed){										//上一次命令对于除MT_Speed以外的其他模式，最终状态都是停止
			if (time - m_lastSendCmd.time>1000 && robotSensor.isRobotStopped()){		//通过判断停止来判断命令是否完成
				printf_s("last command %s done, time=%d, m_lastSendCmdTime=%d!\n", m_lastSendCmd.movetypeName.c_str(), time, m_lastSendCmd.time);
				return true;
			}
			if (time - m_lastSendCmd.time > 2000){										//命令执行超时，可能命令没有被执行，重发
				printf_s("time=%d m_lastSendCmdTime=%d\n", time, m_lastSendCmd.time);
				m_lastSendCmd.cmdID--;
				m_lastSendCmd.time = m_currCmd.time = time;
			}
			return false;
		}
		else{																		//上一条命令是速度模式，当持续时间超过指定时间才认为结束
			if ((m_lastSendCmd.continueTime != 0) && (time - m_lastSendCmd.time < m_lastSendCmd.continueTime)){
				return false;
			}
			return true;
		}
	}
}

void SerialThreadWrite::printSetMovetype(const ControlCmd& currCmd){
	printf_s("Set movetype %s, param1=%d, param2=%d\n", currCmd.movetypeName.c_str(), currCmd.param1, currCmd.param2);
}
void SerialThreadWrite::pushWriteBytes(vecByte& writeBytes){
	{
		CAutoLock autoLock(&m_writeBytesLock);
		m_writeBytes = writeBytes;
	}
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
