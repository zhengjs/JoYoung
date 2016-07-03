#pragma once

#include "JoyoungRobot.h"
#include "AutoLock.h"

class Sensor{
public:
	Variables_MotorEncoder mEncoder;
	Variables_Bump		   mBump;
	Variables_Infrared     mInfrared;
	int					   mnUltrasonic[4];		//mm
	Variables_WheelDrop    mWheelDrop;
	bool mbNewEncoder;
	bool mbNewBump;
	bool mbNewInfrared;
	bool mbNewUltrasonic;
	bool mbNewWheelDrop;

	Sensor() :mbNewEncoder(false), mbNewBump(false), mbNewInfrared(false), mbNewUltrasonic(false), mbNewWheelDrop(false){
		memset(&mEncoder, 0, sizeof(mEncoder));
		memset(&mBump, 0, sizeof(mBump));
		memset(&mInfrared, 0, sizeof(mInfrared));
		memset(mnUltrasonic, 0, sizeof(mnUltrasonic));
		memset(&mWheelDrop, 0, sizeof(mWheelDrop));
		memset(&mLastEncoder, 0, sizeof(mLastEncoder));
	}
	bool setSensorValues(const SensorType sensorType, const int sensorIndex, \
						const LPVOID sensorReportData, const int sensorReportSize)
	{
		if (sensorReportSize <= 0)
			return false;
		switch (sensorType){
		case ST_MotorEncoder:
			{
				CAutoLock autoLock(&mLockEncoder);
				mLastEncoder = mEncoder;							//保存上一次的编码器值，用于判断机器人是否停止，进而判断命令是否执行完
				Variables_MotorEncoder* encoderVars = (Variables_MotorEncoder*)sensorReportData;
				mEncoder = *encoderVars;
				mbNewEncoder = true;
				break;
			}
		case ST_Bump:
			{
				CAutoLock autoLock(&mLockBump);
				Variables_Bump* bumpVars = (Variables_Bump*)sensorReportData;
				mBump = *bumpVars;
				mbNewBump = true;
				break;
			}
		case ST_Infrared:
			{
				CAutoLock autoLock(&mLockInfrared);
				Variables_Infrared* infraredVars = (Variables_Infrared*)sensorReportData;
				mInfrared = *infraredVars;
				mbNewInfrared = true;
				break;
			}
		case ST_WheelDrop:
			{
				CAutoLock autoLock(&mLockWheelDrop);
				Variables_WheelDrop* wheelDropVars = (Variables_WheelDrop*)sensorReportData;
				mWheelDrop = *wheelDropVars;
				mbNewWheelDrop = true;
				break;
			}
		case ST_Cliff:
			{
				 break;
			}
		case ST_Ultrasonic:      //超声波
			{
				CAutoLock autoLock(&mLockUltrasonic);
				Variables_Ultrasonic* ultrasonicVars = (Variables_Ultrasonic*)sensorReportData;
				mnUltrasonic[ultrasonicVars->nIndex] = ultrasonicVars->nDistanceMM;
				mbNewUltrasonic = true;
				break;
			}
		default:
			return false;
		}
		return true;
	}
	bool isRobotStopped(){
		if (mLastEncoder.leftMotor == mEncoder.leftMotor && mLastEncoder.rightMotor == mEncoder.rightMotor && mLastEncoder.stamp < mEncoder.stamp){
			return true;
		}
		return false;
	}
private:
	Lock_	mLockEncoder;
	Lock_	mLockBump;
	Lock_	mLockInfrared;
	Lock_	mLockUltrasonic;
	Lock_	mLockWheelDrop;

	Variables_MotorEncoder mLastEncoder;
};

extern Sensor robotSensor;