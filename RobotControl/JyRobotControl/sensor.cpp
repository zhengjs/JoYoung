#include "stdafx.h"
#include "sensor.h"

Sensor robotSensor;


void Sensor::processSerialAdReadBytes(vecByte& reportBuffer)
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
			//sensorVariablesChanged(m_pReportProcParam, SensorType::ST_Ultrasonic, 0, (LPVOID)&(row.param.ultrasonic), sizeof(row.param.ultrasonic));
			break;
		}
	}
}


void Sensor::processSerialJyReadBytes(vecByte& reportBuffer)
{
	{
		CAutoLock autoLock(&m_lockReportBuffer);
		m_JYReportBuffer.insert(m_JYReportBuffer.end(), reportBuffer.begin(), reportBuffer.end());
		m_newReportData = true;
	}
	return;
}

void Sensor::Run(Thread_* pThread){
	size_t nOldRowSize;

	do{
		if (pThread->wantStop(m_nPeriod))
			continue;
		//printf_s("sensor thread runing!\n");
		{
			CAutoLock autoLock1(&m_lockReportBuffer);
			nOldRowSize = m_serialJyReadRows.size();
			if (m_JYReportBuffer.empty())
				continue;
			CAutoLock autoLock2(&m_lockRows);
			m_jyProtocol.getRowsWithBytes(m_JYReportBuffer, m_serialJyReadRows);
		}

		for (auto i = nOldRowSize; i < m_serialJyReadRows.size(); ++i)
		{
			const JoyoungRow& row = m_serialJyReadRows[i];
			switch (row.type)
			{
			case JoyoungRowType::RRT_MotorEncoder:
				setSensorValues(ST_MotorEncoder, 0, (LPVOID)&(row.param.motorEncoder), sizeof(row.param.motorEncoder));
				break;
			case JoyoungRowType::RRT_Bump:
				setSensorValues(ST_Bump, 0, (LPVOID)&(row.param.bump), sizeof(row.param.bump));
				//printf_s("bump data: %d %d\n", robotSensor.mBump.leftBump, robotSensor.mBump.rightBump);
				break;
			case JoyoungRowType::RRT_Infrared:
				setSensorValues(ST_Infrared, 0, (LPVOID)&(row.param.infrared), sizeof(row.param.infrared));
				printf_s("Infrared data: %d %d %d %d %d \n", mInfrared.infraredL1, mInfrared.infraredL2, \
					mInfrared.infraredC, mInfrared.infraredR2, mInfrared.infraredR1);
				break;
			case JoyoungRowType::RRT_WheelDrop:
				setSensorValues(ST_WheelDrop, 0, (LPVOID)&(row.param.drop), sizeof(row.param.drop));
				break;
			default:
				break;
			}
		}

	} while (true);

	printf_s("sensor thread stop!\n");
}