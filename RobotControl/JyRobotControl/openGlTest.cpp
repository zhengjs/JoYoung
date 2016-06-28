// openGlTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "encoderPathDrawer.h"

#include <windows.h>
/*
*    第一个OpenGL程序.
*/
#define GLUT_DISABLE_ATEXIT_HACK  
#include<gl/GLUT.H>  
#include<gl/GLU.H>  
#include<gl/GL.H>
#include <math.h>  

void init(void)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.4, 0.4, 0.4, 0.0);		//set the background color
	glClear(GL_COLOR_BUFFER_BIT);
}

EncoderPathDrawer pathDrawer;
int spinX=0, spinY=0;

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
	Point currentPos;
	float regionLength, regionWidth;
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

void idlefunc(){
	glutPostRedisplay();
	Sleep(5);
}

void mouseMove(int x, int y){
	static int lastX, lastY;
	int dx = x - lastX;
	int dy = y - lastY;
	if (abs(dx) < 5 && abs(dy) < 5){
		spinX += dx;
		spinY += dy;
		printf_s("spinX=%d, spinY=%d\n", spinX, spinY);
		glutPostRedisplay();
	}
	lastX = x;
	lastY = y;
}

int main(int argc, char *argv[])
{
	pathDrawer.setRegion(100, 100);
	pathDrawer.addPoint(40, 0, .0);
	pathDrawer.addPoint( 0, 30, .0);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 400);
	glutCreateWindow("Encoder path");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);
	glutMotionFunc(mouseMove);
	glutMainLoop();//进行消息循环 

	return 0;
}

