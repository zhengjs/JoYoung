// draw the path computed from encoder
#include "stdafx.h"
//#include "GL/glut.h"
#include "GL/freeglut.h"
#include<math.h>
#include "encoderPathDrawer.h"
#include "sensor.h"

const GLfloat Pi = 3.1415926536f;
EncoderPathDrawer pathDrawer;
float spinX = 0, spinY = 0;


void EncoderPathDrawer::setRegion(const float& length, const float& width){
	regionLength = length;
	regionWidth = width;
}

void EncoderPathDrawer::addPoint(const float& x, const float& y, const float&z){
	if (abs(x) > 0.5*regionLength || abs(y) > 0.5*regionWidth){
		//printf_s("Point (%f, %f, %f) out of bounds.\n", x, y, z);
		//return;
	}
	Pointf newPoint(x, y, .0f);
	realPath.push_back(newPoint);
	float x_ = (float)x / regionLength * 1.5f, y_ = (float)y / regionWidth * 1.5f;				//绘制出的轨迹最长和最宽占窗口的3/4
	Pointf normalizedPoint(x_, y_, .0f);
	path.push_back(normalizedPoint);
	pathUpdate = true;
}

void EncoderPathDrawer::addPoint(const Pointf& newPoint){
	addPoint(newPoint.x, newPoint.y, newPoint.z);
}

void EncoderPathDrawer::drawRobot(){
	int n = 50;
	GLfloat R = 0.08f;

	glPushMatrix();
	glColor3f( 250.0/255, 128.0/255, 114.0/255);
	int pathSize = path.size()-1;
	glTranslated(path[pathSize].x, path[pathSize].y, 0);
	glBegin(GL_POLYGON);//OpenGL要求：指定顶点的命令必须包含在glBegin函数之后，  
						//glEnd函数之前（否则指定的顶点将被忽略）。并由glBegin来指明如何使用这些点  
						//GL_POLYGON表示画多边形（由点连接成多边形）
	for (size_t i = 0; i<n; ++i)
		glVertex3f(R*cos(2 * Pi / n*i), R*sin(2 * Pi / n*i), 0);
	glEnd();

	glRotatef(-yaw/Pi*180, 0, 0, 1);
	Variables_Infrared infraredVars;
	{
		CAutoLock autoLock(&robotSensor.mLockInfrared);
		infraredVars = robotSensor.mInfrared;
	}
	glPointSize(4.0f);
	glColor3f(0, 0.6, 0.5);
	glBegin(GL_POINTS);//OpenGL要求：指定顶点的命令必须包含在glBegin函数之后，  
	//glEnd函数之前（否则指定的顶点将被忽略）。并由glBegin来指明如何使用这些点  
	//GL_POLYGON表示画多边形（由点连接成多边形）
	R = 0.09f;
	if (infraredVars.infraredR1){
		glColor3f(1, 0, 0);
	}
	else{
		glColor3f(0, 0.6, 0.5);
	}
	glVertex3f(R*cos(Pi / 6), R*sin(Pi / 6), 0);

	if (infraredVars.infraredR2){
		glColor3f(1, 0, 0);
	}
	else{
		glColor3f(0, 0.6, 0.5);
	}
	glVertex3f(R*cos(Pi / 3), R*sin(Pi / 3), 0);

	if (infraredVars.infraredC){
		glColor3f(1, 0, 0);
	}
	else{
		glColor3f(0, 0.6, 0.5);
	}
	glVertex3f(0, R, 0);

	if (infraredVars.infraredL2){
		glColor3f(1, 0, 0);
	}
	else{
		glColor3f(0, 0.6, 0.5);
	}
	glVertex3f(R*cos(Pi * 4 / 6), R*sin(Pi * 4 / 6), 0);

	if (infraredVars.infraredL1){
		glColor3f(1, 0, 0);
	}
	else{
		glColor3f(0, 0.6, 0.5);
	}
	glVertex3f(R*cos(Pi * 5 / 6), R*sin(Pi * 5 / 6), 0);

	R = 0.07f;
	n = 20;
	Variables_Bump bumpVars;
	{
		CAutoLock autoLock(&robotSensor.mLockBump);
		bumpVars = robotSensor.mBump;
	}
	if (bumpVars.rightBump)
		glColor3f(1, 0, 0);
	else
		glColor3f(0, 0.8, 0.4);
	for (size_t i = 0; i < n; i++){
		glVertex3f(R*cos(Pi*i / 2.2 / n), R*sin(Pi*i / 2.2 / n), 0);
	}

	if (bumpVars.leftBump)
		glColor3f(1, 0, 0);
	else
		glColor3f(0, 0.8, 0.4);
	for (size_t i = 0; i < n; i++){
		glVertex3f(-R*cos(Pi*i / 2.2 / n), R*sin(Pi*i / 2.2 / n), 0);
	}
	glEnd();

	glPopMatrix();
}

void EncoderPathDrawer::drawPath(){
	glTranslatef(0, 0, 1);
	glRotatef(spinX, 0, 1, 0);
	glRotatef(spinY, 1, 0, 0);
	int pathSize = path.size()-1;
	gluLookAt(path[pathSize].x, path[pathSize].y, 1, path[pathSize].x, path[pathSize].y, 0, sin(yaw*Pi / 180), cos(yaw*Pi / 180), 0);
	//glOrtho(-2, 2, -2, 2, -2, 2);

	glLineWidth(1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	for (int i = -15; i <= 15; i++){
		glVertex3f(-3.0f, 0.2*i, .0f);
		glVertex3f(3.0f, 0.2*i, .0f);
		glVertex3f(0.2*i, -3.0f, .0f);
		glVertex3f(0.2*i, 3.0f, .0f);
	}
	glEnd();

	glLineWidth(2.8f);
	glColor3f(.6f, .6f, 1.0f);
	glBegin(GL_LINES);
	for (size_t i = 1, iend = path.size(); i < iend; i++){
		glVertex3f(path[i - 1].x, path[i - 1].y, path[i - 1].z);
		glVertex3f(path[i].x, path[i].y, path[i].z);
	}
	glEnd();

	drawRobot();
}

void EncoderPathDrawer::getCurrentPos(Pointf& curPos){
	if (realPath.size()){
		//curPos = realPath.back();
		curPos = *(realPath.begin()+realPath.size()-1);
	}
	else{
		curPos.x = .0f;
		curPos.y = .0f;
		curPos.z = .0f;
	}
	return;
}

void EncoderPathDrawer::createPathPoint(int motorEncoderL, int motorEncoderR){
	static int lastEncoderL=0, lastEncoderR=0;
	if (lastEncoderL == 0 && lastEncoderR == 0){									//receive the encoder data for the first time
		lastEncoderL = motorEncoderL;
		lastEncoderR = motorEncoderR;
		return;
	}
	if (lastEncoderL == motorEncoderL && lastEncoderR == motorEncoderR){			//robot does not move
		return;
	}
	float deltaL = motorEncoderL - lastEncoderL, deltaR = motorEncoderR - lastEncoderR;
	lastEncoderL = motorEncoderL;
	lastEncoderR = motorEncoderR;
	deltaL *= 0.2f;						//convert to mm
	deltaR *= 0.2f;						//convert to mm
	//printf_s("deltaL=%f, deltaR=%f ", deltaL, deltaR);

	Pointf currentPos;
	getCurrentPos(currentPos);

	currentPos.x += (deltaL + deltaR)*0.5f*sin(yaw);
	currentPos.y += (deltaL + deltaR)*0.5f*cos(yaw);
	yaw += (deltaL - deltaR)/b;
	//printf_s("currentX=%f, currentY=%f, yaw=%f \n", currentPos.x, currentPos.y, yaw);
	addPoint(currentPos);
}


void encoderDataChangedProc(LPVOID pProcParam, const SensorType sensorType, const int sensorIndex, const LPVOID sesorReportData, const int sesorReportSize){
	if (ST_MotorEncoder != sensorType)
		return;
	if (sizeof(Variables_MotorEncoder) != sesorReportSize)
		return;

	Variables_MotorEncoder* encoderVars = (Variables_MotorEncoder*)sesorReportData;
	//printf_s("encoderL=%d, encoderR=%d, timestamp=%d \n", encoderVars->leftMotor, encoderVars->rightMotor, encoderVars->stamp);				//encoder data update frequency: 100Hz
	pathDrawer.createPathPoint(encoderVars->leftMotor, encoderVars->rightMotor);
}


void display(){
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	//glPushMatrix();
	//glPopMatrix();
	pathDrawer.drawPath();
	glFlush();								//command should to be done right now
}

void keyPressed(unsigned char key, int x, int y){
	Pointf currentPos;
	pathDrawer.getCurrentPos(currentPos);
	float regionLength, regionWidth;
	pathDrawer.getRegion(regionLength, regionWidth);

	bool newPoint = true;
	switch (key){
	case 'a':				//left
		currentPos.x -= regionLength*0.02;
		break;
	case 'w':				//up
		currentPos.y += regionWidth*0.02;
		break;
	case 'd':				//right
		currentPos.x += regionLength*0.02;
		break;
	case 's':				//down
		currentPos.y -= regionWidth*0.02;
		break;
	case 'r':
		spinX = 0;
		spinY = 0;
		newPoint = false;
		break;
	default:
		newPoint = false;
		break;
	}
	if (newPoint){
		pathDrawer.addPoint(currentPos);
	}
	glutPostRedisplay();
}

void mouseMove(int x, int y){
	static int lastX, lastY;
	int dx = x - lastX;
	int dy = y - lastY;
	if (abs(dx) < 5 && abs(dy) < 5){
		spinX += dx;
		spinY += dy;
		//printf_s("spinX=%d, spinY=%d\n", spinX, spinY);
		glutPostRedisplay();
	}
	lastX = x;
	lastY = y;
}

int initPathDrawer()
{
	pathDrawer.setRegion(8000.0f, 8000.0f);			//region: 4*4m
	pathDrawer.addPoint(0, 0, 0);
	int argc = 1;
	char s[] = "";
	char *argv[] = { s };
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 400);
	glutCreateWindow("Encoder path");
	glShadeModel(GL_SMOOTH);
	glClearColor(0.4, 0.4, 0.4, 0.0);			//set the background color
	glClear(GL_COLOR_BUFFER_BIT);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);
	glutMotionFunc(mouseMove);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);		//设置opengl关闭后，仍可以执行后续代码
	//glutMainLoop();							//进行消息循环,调用该函数程序将进入死循环，后续代码无法执行

	return 0;
}

