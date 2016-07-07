#pragma once

#include <windows.h>
#include <string>

typedef enum
{
	ST_MotorEncoder	=1,	//马达编码器
	ST_Bump,			//碰撞
	ST_Infrared,        //红外，todo,不知道干嘛j,要么是用于悬崖，要么是用于充电引导，要么是用于避障
	ST_WheelDrop,		//Wheel Dropped(车轮已落下)wheel raised(车轮已抬起)
	ST_Cliff,			//悬崖（防跌落）
    
    ST_Ultrasonic,      //超声波
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
	MT_Speed,		//速度模式 Param1:左轮速度（85mm/s~340mm/s）			Param2:右轮速度（85mm / s~340mm / s）
	MT_Distance,	//里程模式 Param1:目标里程（+0xFFFF mm ~ -0xFFFF mm）	Param2:速度（85mm / s ~340mm / s）
	MT_Angle,		//角度模式 Param1:目标角度（+0xFFFF 度 ~ -0xFFFF 度）	Param2:角速度（[+-]37度 / s~148度 / s）
	MT_NONE = 4
}
MoveType;

#define Min_MoveSpeed 85
#define Max_MoveSpeed 340

class MovingPlanManager;

class JoyoungRobot
{
public:
    static JoyoungRobot* connectRobot(const UINT& serialJyPort, const UINT& serialJyRate, const UINT& serialUlPort, const UINT& serialUlRate);
    static void disconnectRobot(JoyoungRobot* robot);

    virtual ~JoyoungRobot(void){ ; }

    virtual MovingPlanManager* movingPlanManager() =0;

	virtual int		setMoveType(const MoveType& moveType, const int& moveParam1, const int& moveParam2) =0;
	virtual void    getLastSetMoveType(MoveType& moveType, int& moveParam1, int& moveParam2) = 0;

    typedef void    (*SensorVariablesChangedCallbackProc)(LPVOID pProcParam, const SensorType sensorType, const int sensorIndex, const LPVOID sesorReportData, const int sesorReportSize);
	virtual bool    setSensorVariablesChangedCallbackProc(SensorVariablesChangedCallbackProc pProc, LPVOID pProcParam, const int periodMS =10) = 0;
};

struct Variables_MotorEncoder{
	DWORD leftMotor;
	DWORD rightMotor;
	DWORD stamp;
};

struct Variables_Bump{
	BOOL leftBump;
	BOOL rightBump;
};

struct Variables_Infrared{
	BOOL infraredL1;
	BOOL infraredL2;
	BOOL infraredC;
	BOOL infraredR2;
	BOOL infraredR1;
};

struct Variables_Ultrasonic{
    int nIndex;
    int nDistanceMM;
};

struct Variables_WheelDrop
{
	BOOL wheelDrop;
};

#define StatusReportPeriod_MinMS	10


BYTE getBytes_Xor(const LPBYTE pStart, const LPBYTE pEnd);


typedef enum{
	CMD_TYPE_DEFAULT = 1,	//don't wait, may cover the last command
	CMD_TYPE_NOWAIT,		//
	CMD_TYPE_BLOCK,			//线程被阻塞直到释放（上一次动作完成）
	CMD_TYPE_NONBLOCK		//如果上一次动作没有完成，则函数立刻返回，本次命令被放弃
}CommandType;
/************************************************************
used to record the command sent to the cleaner
************************************************************/
typedef struct{
	MoveType	moveType;
	std::string	movetypeName;
	int			param1;
	int			param2;
	DWORD		time;					//命令发出的时间，ms
	DWORD		continueTime;			//命令需要持续的时间，一般用于速度模式，ms
	DWORD		cmdID;
	CommandType cmdType;
}ControlCmd;
