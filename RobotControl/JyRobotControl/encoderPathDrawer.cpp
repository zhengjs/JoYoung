// draw the path computed from encoder
#include "stdafx.h"
//#include "GL/glut.h"
#include "GL/freeglut.h"
#include<math.h>
#include "encoderPathDrawer.h"

void EncoderPathDrawer::setRegion(const float& length, const float& width){
	regionLength = length;
	regionWidth = width;
}

void EncoderPathDrawer::addPoint(const float& x, const float& y, const float&z){
	if (abs(x) > 0.5*regionLength || abs(y) > 0.5*regionWidth){
		//printf_s("Error! Point (%d, %d, %d) out of bounds.\n", x, y, z);
		return;
	}
	Pointf newPoint(x, y, .0f);
	realPath.push_back(newPoint);
	float x_ = (float)x / regionLength * 1.5f, y_ = (float)y / regionWidth * 1.5f;				//���Ƴ��Ĺ켣������ռ���ڵ�3/4
	Pointf normalizedPoint(x_, y_, .0f);
	path.push_back(normalizedPoint);
	pathUpdate = true;
}

void EncoderPathDrawer::addPoint(const Pointf& newPoint){
	addPoint(newPoint.x, newPoint.y, newPoint.z);
}

void EncoderPathDrawer::drawPath(){
	glLineWidth(2.8f);
	glColor3f(.6f, .6f, 1.0f);
	glBegin(GL_LINES);
	for (size_t i = 1, iend = path.size(); i < iend; i++){
		glVertex3f(path[i - 1].x, path[i - 1].y, path[i - 1].z);
		glVertex3f(path[i].x, path[i].y, path[i].z);
	}

	glEnd();
}

void EncoderPathDrawer::getCurrentPos(Pointf& curPos){
	if (realPath.size()){
		curPos = realPath.back();
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


EncoderPathDrawer pathDrawer;
int spinX = 0, spinY = 0;

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
	glRotatef(spinX, 0, 1, 0);
	glRotatef(spinY, 1, 0, 0);
	pathDrawer.drawPath();
	//glPopMatrix();
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
	glutInitWindowSize(800, 800);
	glutCreateWindow("Encoder path");
	glShadeModel(GL_SMOOTH);
	glClearColor(0.4, 0.4, 0.4, 0.0);			//set the background color
	glClear(GL_COLOR_BUFFER_BIT);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);
	glutMotionFunc(mouseMove);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);		//����opengl�رպ��Կ���ִ�к�������
	//glutMainLoop();							//������Ϣѭ��,���øú������򽫽�����ѭ�������������޷�ִ��

	return 0;
}

