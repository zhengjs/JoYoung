#pragma once

#include <vector>
#include "JoyoungRobot.h"
#include "Thread.h"
#include "GL/freeglut.h"

template<typename T>
struct Point{
	T x, y, z;
	Point(const T& x_ = 0, const T& y_ = 0, const T& z_ = 0){
		x = x_;
		y = y_;
		z = z_;
	}
};
typedef Point<int> Pointi;
typedef Point<float> Pointf;


class EncoderPathDrawer{
public:
	EncoderPathDrawer() :yaw(.0f), regionLength(.0f), regionWidth(.0f), b(270.0f), pathUpdate(false){}

	void setRegion(const float& length, const float& width);
	void getRegion(float& length, float& width){ length = regionLength; width = regionWidth; return; }
	void addPoint(const float& x, const float& y, const float&z);
	void addPoint(const Pointf& newPoint);
	void drawRobot();
	void drawPath();
	void getCurrentPos(Pointf& curPos);
	void createPathPoint(int motorEncoderL, int motorEncoderR);
	bool pathUpdate;

private:
	std::vector<Pointf> realPath;			//real position in world coordinate system (mm)
	std::vector<Pointf> path;
	float regionLength, regionWidth;		//mm

	float yaw;
	const float b;							//轮距, 270mm
};

extern int initPathDrawer();
extern EncoderPathDrawer pathDrawer;
extern void encoderDataChangedProc(LPVOID pProcParam, const SensorType sensorType, const int sensorIndex, const LPVOID sesorReportData, const int sesorReportSize);

/****************************************************************************
openGl thread
类Thread_封装了创建线程及执行线程函数的一系列操作，当需要创建一个线程执行特定操作时，
只需要创建该操作的类，并实现ThreadProc_接口，复写Run方法，定义一个Thread_成员，最后
在合适的时刻调用Thread_成员的Start方法，线程即启动
******************************************************************************/
class JoyoungRobotImp;

class OpenglThread :public ThreadProc_
{
public:
	OpenglThread(JoyoungRobotImp* pRobot) :ThreadProc_(), m_pRobot(pRobot)
	{
		if (!m_Thread_.Start(this))
			return;
	}
protected:
	void Run(Thread_* pThread) override{
		initPathDrawer();
		do{
			glutPostRedisplay();
			//printf_s("opengl thread runing!\n");
			glutMainLoopEvent();
			Sleep(10);
		} while (true);
	}
private:
	Thread_ m_Thread_;
	JoyoungRobotImp* m_pRobot;
};