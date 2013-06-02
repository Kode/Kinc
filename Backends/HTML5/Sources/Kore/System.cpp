#include "pch.h"
#include <Kore/System.h>
#include <cstring>
#include <Kore/Files/File.h>
#include <Kore/Application.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/KeyEvent.h>
#include <Kore/Input/Mouse.h>
#include <Kore/ogl.h>

namespace {
	int argc;
	char** argv;

	void idlefunc() {

	}

	void drawfunc() {
		Kore::Application::the()->callback();
	}
}

using namespace Kore;

void* System::createWindow() {
	glutInit(&argc, argv);
	glutInitWindowSize(300, 300);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Kore");
	glutIdleFunc(idlefunc);
	//glutReshapeFunc(gears_reshape);
	glutDisplayFunc(drawfunc);
	//glutSpecialFunc(gears_special);
	
	return nullptr;
}

bool System::handleMessages() {
	return true;
}

void Kore::System::swapBuffers() {

}

void Kore::System::destroyWindow() {

}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {

}

void Kore::System::setTitle(const char* title) {

}

void Kore::System::showWindow() {

}

Kore::System::ticks Kore::System::getFrequency() {
	return 1000000;
}

Kore::System::ticks Kore::System::getTimestamp() {
	return 0;
}

extern int kore(int argc, char** argv);

int main(int argc, char** argv) {
	::argc = argc;
	::argv = argv;
	kore(argc, argv);
	System::createWindow();
	glutMainLoop();
}

