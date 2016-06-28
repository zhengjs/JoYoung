// draw the path computed from encoder
#include "stdafx.h"
//#include "GL/glut.h"
#include "GL/freeglut.h"
#include<math.h>
#include "encoderPathDrawer.h"

void EncoderPathDrawer::setRegion(const int& length, const int& width){
	regionLength = length;
	regionWidth = width;
}

void EncoderPathDrawer::addPoint(const int& x, const int& y, const int&z){
	if (abs(x) > 0.5*regionLength || abs(y) > 0.5*regionWidth){
		printf_s("Error! Point (%f, %f, %f) out of bounds.\n", x, y);
		return;
	}
	Pointi newPoint(x, y, 0);
	originalPath.push_back(newPoint);
	float x_ = (float)x / regionLength * 1.5f, y_ = (float)y / regionWidth * 1.5f;				//绘制出的轨迹最长和最宽占窗口的3/4
	Pointf normalizedPoint(x_, y_, .0f);
	path.push_back(normalizedPoint);
}

void EncoderPathDrawer::addPoint(const Pointi& newPoint){
	addPoint(newPoint.x, newPoint.y, newPoint.z);
}

void EncoderPathDrawer::drawPath(){
	glLineWidth(1.8f);
	glColor3f(.6f, .6f, 1.0f);
	glBegin(GL_LINES);
	for (size_t i = 1, iend = path.size(); i < iend; i++){
		glVertex3f(path[i - 1].x, path[i - 1].y, path[i - 1].z);
		glVertex3f(path[i].x, path[i].y, path[i].z);
	}

	glEnd();
}

void EncoderPathDrawer::getCurrentPos(Pointi& curPos){
	if (originalPath.size()){
		curPos = originalPath.back();
	}
	else{
		curPos.x = 0;
		curPos.y = 0;
		curPos.z = 0;
	}
	return;
}


EncoderPathDrawer pathDrawer;
int spinX = 0, spinY = 0;

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
	Pointi currentPos;
	int regionLength, regionWidth;
	pathDrawer.getRegion(regionLength, regionWidth);
	pathDrawer.getCurrentPos(currentPos);

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
	pathDrawer.setRegion(100, 100);
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
	glClearColor(0.4, 0.4, 0.4, 0.0);		//set the background color
	glClear(GL_COLOR_BUFFER_BIT);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);
	glutMotionFunc(mouseMove);
	glutMainLoop();						//进行消息循环,调用该函数程序将进入死循环，后续代码无法执行

	return 0;
}

