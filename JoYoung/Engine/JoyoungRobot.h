#pragma once

#include <windows.h>

typedef enum
{
	ST_MotorEncoder	=1,	//马达编码器
	ST_Bump,			//碰撞
	ST_Infrared,        //红外，todo,不知道干嘛j,要么是用于悬崖，要么是用于充电引导，要么是用于避障
	ST_WheelDrop,		//Wheel Dropped(车轮已落下)wheel raised(车轮已抬起)
	ST_Cliff,			//悬崖（防跌落）
}
SensorType;

/************************************************************************
移动模式：
1.不进入停止模式，不可更改为其它移动模式

可选的移动模式：
1.停止模式

2.速度模式
	2.1.参数
		Param1:左轮速度（85mm/s~340mm/s）
		Param2:右轮速度（85mm/s~340mm/s）
	2.2.->停止模式
		2.2.1.速度模式设置指令（Param1=0,Param2=0）
3.里程模式
	3.1.参数
		Param1:目标里程（+0xFFFF mm ~ -0xFFFF mm）
		Param2:速度（85mm/s ~ 340mm/s）
	3.2.->停止模式
		3.2.2.执行完成
		3.2.1.速度模式设置指令（Param1=0,Param2=0）
4.角度模式
	4.1.参数
		Param1:目标角度（+0xFFFF 度 ~ -0xFFFF 度）
		Param2:角速度（[+-]37度/s~148度/s）
	4.2.->停止模式
		4.2.2.执行完成
		4.2.1.速度模式设置指令（Param1=0,Param2=0）

************************************************************************/

typedef enum
{
	MT_Stop		=0, //停止模式
	MT_Speed,		//速度模式
	MT_Distance,	//里程模式
	MT_Angle,		//角度模式
}
MoveType;


class JoyoungRobot
{
public:
	static JoyoungRobot* getSingleton(const UINT& serialPort, const UINT& serialRate);
	virtual ~JoyoungRobot(void){;}

	virtual int		setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2) =0;
	virtual void    getLastSetMoveType(MoveType& moveType, int& moveParam1, int& moveParam2) = 0;

	typedef void	(*SensorReportCallbackProc)(LPVOID pProcParam, const SensorType& sensorType, const int& sensorIndex, const LPVOID& sesorReportData, const int& sesorReportSize);
	virtual void    setStatusReportCallbackProc(SensorReportCallbackProc pProc, LPVOID pProcParam, const int& periodMS) = 0;
};

#define StatusReportPeriod_MinMS	10