#include "pch.h"
#include <Kore/System.h>
#include <cstring>
#include <Kore/Application.h>
#include <Kore/Audio/Audio.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/ogl.h>
#include <stdlib.h>
#include <GL/glfw.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

namespace {
	int argc;
	char** argv;

	//void idlefunc() {
	//	glutPostRedisplay();
	//}

	void drawfunc() {
		Kore::Application::the()->callback();
		Kore::Audio::update();
		//glutSwapBuffers();
		glfwSwapBuffers();
	}

	/*void specialfunc(int key, int x, int y) {
		switch (key) {
		case GLUT_KEY_LEFT:
			Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Left));
			break;
		case GLUT_KEY_RIGHT:
			Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Right));
			break;
		case GLUT_KEY_UP:
			Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Up));
			break;
		case GLUT_KEY_DOWN:
			Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Down));
			break;
		}
	}*/
	
	void onKeyPressed(int key, int action) {
		if (action == GLFW_PRESS) {
			switch (key) {
			case 262:
				Kore::Keyboard::the()->_keydown(Kore::Key_Right, ' ');
				break;
			case 263:
				Kore::Keyboard::the()->_keydown(Kore::Key_Left, ' ');
				break;
			case 265:
				Kore::Keyboard::the()->_keydown(Kore::Key_Up, ' ');
				break;
			case 264:
				Kore::Keyboard::the()->_keydown(Kore::Key_Down, ' ');
				break;
			}
		}
		else {
			switch (key) {
			case 262:
				Kore::Keyboard::the()->_keyup(Kore::Key_Right, ' ');
				break;
			case 263:
				Kore::Keyboard::the()->_keyup(Kore::Key_Left, ' ');
				break;
			case 265:
				Kore::Keyboard::the()->_keyup(Kore::Key_Up, ' ');
				break;
			case 264:
				Kore::Keyboard::the()->_keyup(Kore::Key_Down, ' ');
				break;
			}
		}
	}

	int mouseX = 0;
	int mouseY = 0;

	void onMouseClick(int button, int action) {
		if (action == GLFW_PRESS) {
			if (button == 0) Kore::Mouse::the()->_press(0, mouseX, mouseY);
			else if (button == 1) Kore::Mouse::the()->_press(1, mouseX, mouseY);
		}
		else {
			if (button == 0) Kore::Mouse::the()->_release(0, mouseX, mouseY);
			else if (button == 1) Kore::Mouse::the()->_release(1, mouseX, mouseY);
		}
	}

	void onMouseMove(int x, int y) {
		mouseX = x;
		mouseY = y;
		Kore::Mouse::the()->_move(x, y);
	}
}

using namespace Kore;

void* System::createWindow() {
	/*glutInit(&argc, argv);
	glutInitWindowSize(Kore::Application::the()->width(), Kore::Application::the()->height());
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Kore");
	glutIdleFunc(idlefunc);
	//glutReshapeFunc(reshapefunc);
	glutDisplayFunc(drawfunc);
	glutSpecialFunc(specialfunc);*/

	glfwInit();
	glfwOpenWindow(Kore::Application::the()->width(), Kore::Application::the()->height(), 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle(Kore::Application::the()->name());
 
	glfwSetKeyCallback(onKeyPressed);
	//glfwSetCharCallback(onCharPressed);
	//glfwSetWindowCloseCallback(onClose);
	//glfwSetWindowSizeCallback(onResize);
	//glfwSetWindowRefreshCallback(onRefresh);
	//glfwSetMouseWheelCallback(onMouseWheel);
	glfwSetMousePosCallback(onMouseMove);
	glfwSetMouseButtonCallback(onMouseClick);

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

void Kore::System::setKeepScreenOn( bool on ) {
    
}

void Kore::System::showWindow() {

}

double Kore::System::frequency() {
	return 1000.0;
}

int Kore::System::screenWidth() {
    return 800;
}

int Kore::System::screenHeight() {
    return 600;
}

int Kore::System::desktopWidth() {
    return 800;
}

int Kore::System::desktopHeight() {
    return 600;
}

Kore::System::ticks Kore::System::timestamp() {
	return static_cast<Kore::System::ticks>(glfwGetTime() * 1000.0);
}

double Kore::System::time() {
	//printf("Time: %f\n", glfwGetTime());
	return glfwGetTime();
}

extern int kore(int argc, char** argv);

int main(int argc, char** argv) {
	::argc = argc;
	::argv = argv;
	kore(argc, argv);
	//System::createWindow();
	//glutMainLoop();
	emscripten_set_main_loop(drawfunc, 0, 1);
}

