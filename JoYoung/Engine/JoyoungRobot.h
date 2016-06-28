#pragma once

#include <windows.h>

typedef enum
{
	ST_MotorEncoder	=1,	//��������
	ST_Bump,			//��ײ
	ST_Infrared,        //���⣬todo,��֪������j,Ҫô���������£�Ҫô�����ڳ��������Ҫô�����ڱ���
	ST_WheelDrop,		//Wheel Dropped(����������)wheel raised(������̧��)
	ST_Cliff,			//���£������䣩
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
	MT_Speed,		//�ٶ�ģʽ
	MT_Distance,	//���ģʽ
	MT_Angle,		//�Ƕ�ģʽ
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