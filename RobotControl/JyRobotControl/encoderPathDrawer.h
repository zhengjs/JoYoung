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

/****************************************************************************
openGl thread
类Thread_封装了创建线程及执行线程函数的一系列操作，当需要创建一个线程执行特定操作时，
只需要创建该操作的类，并实现ThreadProc_接口，复写Run方法，定义一个Thread_成员，最后
在合适的时刻调用Thread_成员的Start方法，线程即启动
******************************************************************************/
class EncoderPathDrawer :public ThreadProc_{
public:
	EncoderPathDrawer() :yaw(.0f), regionLength(.0f), regionWidth(.0f), b(270.0f), pathUpdate(false), m_nPeriod(10){}

	void setRegion(const float& length, const float& width);
	void getRegion(float& length, float& width){ length = regionLength; width = regionWidth; return; }
	void addPoint(const float& x, const float& y, const float&z);
	void addPoint(const Pointf& newPoint);
	void drawRobot();
	void drawPath();
	void getCurrentPos(Pointf& curPos);
	void createPathPoint(int motorEncoderL, int motorEncoderR);
	bool pathUpdate;
	void encoderDataChangedProc();

	void init(){
		if (!m_Thread_.Start(this))
			return;
	}
protected:
	void Run(Thread_* pThread) override;

private:
	std::vector<Pointf> realPath;			//real position in world coordinate system (mm)
	std::vector<Pointf> path;
	float regionLength, regionWidth;		//mm

	float yaw;
	const float b;							//轮距, 270mm

	Thread_ m_Thread_;
	int		m_nPeriod;
};


extern EncoderPathDrawer pathDrawer;
//extern int initPathDrawer();

//class JoyoungRobotImp;
//
//class OpenglThread :public ThreadProc_
//{
//public:
//	OpenglThread(JoyoungRobotImp* pRobot) :ThreadProc_()
//	{
//		if (!m_Thread_.Start(this))
//			return;
//	}
//protected:
//	void Run(Thread_* pThread) override;
//private:
//	Thread_ m_Thread_;
//};