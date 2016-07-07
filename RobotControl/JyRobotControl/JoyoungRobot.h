#pragma once

#include <windows.h>
#include <string>

typedef enum
{
	ST_MotorEncoder	=1,	//��������
	ST_Bump,			//��ײ
	ST_Infrared,        //���⣬todo,��֪������j,Ҫô���������£�Ҫô�����ڳ��������Ҫô�����ڱ���
	ST_WheelDrop,		//Wheel Dropped(����������)wheel raised(������̧��)
	ST_Cliff,			//���£������䣩
    
    ST_Ultrasonic,      //������
}
SensorType;

/************************************************************************
�ƶ�ģʽ��
1.������ֹͣģʽ�����ɸ���Ϊ�����ƶ�ģʽ

��ѡ���ƶ�ģʽ��
1.ֹͣģʽ

2.�ٶ�ģʽ
	2.1.����
		Param1:�����ٶȣ�85mm/s~340mm/s��
		Param2:�����ٶȣ�85mm/s~340mm/s��
	2.2.->ֹͣģʽ
		2.2.1.�ٶ�ģʽ����ָ�Param1=0,Param2=0��
3.���ģʽ
	3.1.����
		Param1:Ŀ����̣�+0xFFFF mm ~ -0xFFFF mm��
		Param2:�ٶȣ�85mm/s ~ 340mm/s��
	3.2.->ֹͣģʽ
		3.2.2.ִ�����
		3.2.1.�ٶ�ģʽ����ָ�Param1=0,Param2=0��
4.�Ƕ�ģʽ
	4.1.����
		Param1:Ŀ��Ƕȣ�+0xFFFF �� ~ -0xFFFF �ȣ�
		Param2:���ٶȣ�[+-]37��/s~148��/s��
	4.2.->ֹͣģʽ
		4.2.2.ִ�����
		4.2.1.�ٶ�ģʽ����ָ�Param1=0,Param2=0��

************************************************************************/

typedef enum
{
	MT_Stop		=0, //ֹͣģʽ
	MT_Speed,		//�ٶ�ģʽ Param1:�����ٶȣ�85mm/s~340mm/s��			Param2:�����ٶȣ�85mm / s~340mm / s��
	MT_Distance,	//���ģʽ Param1:Ŀ����̣�+0xFFFF mm ~ -0xFFFF mm��	Param2:�ٶȣ�85mm / s ~340mm / s��
	MT_Angle,		//�Ƕ�ģʽ Param1:Ŀ��Ƕȣ�+0xFFFF �� ~ -0xFFFF �ȣ�	Param2:���ٶȣ�[+-]37�� / s~148�� / s��
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
	CMD_TYPE_BLOCK,			//�̱߳�����ֱ���ͷţ���һ�ζ�����ɣ�
	CMD_TYPE_NONBLOCK		//�����һ�ζ���û����ɣ��������̷��أ������������
}CommandType;
/************************************************************
used to record the command sent to the cleaner
************************************************************/
typedef struct{
	MoveType	moveType;
	std::string	movetypeName;
	int			param1;
	int			param2;
	DWORD		time;					//�������ʱ�䣬ms
	DWORD		continueTime;			//������Ҫ������ʱ�䣬һ�������ٶ�ģʽ��ms
	DWORD		cmdID;
	CommandType cmdType;
}ControlCmd;
