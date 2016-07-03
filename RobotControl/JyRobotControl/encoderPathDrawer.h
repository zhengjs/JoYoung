#pragma once

#include <vector>
#include "JoyoungRobot.h"

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
	const float b;							//ÂÖ¾à, 270mm
};

extern int initPathDrawer();
extern EncoderPathDrawer pathDrawer;
extern void encoderDataChangedProc(LPVOID pProcParam, const SensorType sensorType, const int sensorIndex, const LPVOID sesorReportData, const int sesorReportSize);