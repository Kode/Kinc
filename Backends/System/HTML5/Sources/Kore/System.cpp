#include "pch.h"

#include <GL/glfw.h>
#include <Kore/Audio2/Audio.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/ogl.h>
#include <cstring>
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <stdlib.h>

namespace {
	int argc;
	char** argv;

	// void idlefunc() {
	//	glutPostRedisplay();
	//}

	void drawfunc() {
		Kore::System::callback();
		Kore::Audio2::update();
		// glutSwapBuffers();
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
			case 87:
				Kore::Keyboard::the()->_keydown(Kore::KeyW);
				break;
			case 65:
				Kore::Keyboard::the()->_keydown(Kore::KeyA);
				break;
			case 83:
				Kore::Keyboard::the()->_keydown(Kore::KeyS);
				break;
			case 68:
				Kore::Keyboard::the()->_keydown(Kore::KeyD);
				break;
			case 32:
				Kore::Keyboard::the()->_keydown(Kore::KeySpace);
				break;
			case 262:
				Kore::Keyboard::the()->_keydown(Kore::KeyRight);
				break;
			case 263:
				Kore::Keyboard::the()->_keydown(Kore::KeyLeft);
				break;
			case 265:
				Kore::Keyboard::the()->_keydown(Kore::KeyUp);
				break;
			case 264:
				Kore::Keyboard::the()->_keydown(Kore::KeyDown);
				break;
			}
		}
		else {
			switch (key) {
			case 87:
				Kore::Keyboard::the()->_keyup(Kore::KeyW);
				break;
			case 65:
				Kore::Keyboard::the()->_keyup(Kore::KeyA);
				break;
			case 83:
				Kore::Keyboard::the()->_keyup(Kore::KeyS);
				break;
			case 68:
				Kore::Keyboard::the()->_keyup(Kore::KeyD);
				break;
			case 32:
				Kore::Keyboard::the()->_keyup(Kore::KeySpace);
				break;
			case 262:
				Kore::Keyboard::the()->_keyup(Kore::KeyRight);
				break;
			case 263:
				Kore::Keyboard::the()->_keyup(Kore::KeyLeft);
				break;
			case 265:
				Kore::Keyboard::the()->_keyup(Kore::KeyUp);
				break;
			case 264:
				Kore::Keyboard::the()->_keyup(Kore::KeyDown);
				break;
			}
		}
	}

	int mouseX = 0;
	int mouseY = 0;

	void onMouseClick(int button, int action) {
		if (action == GLFW_PRESS) {
			if (button == 0)
				Kore::Mouse::the()->_press(0, 0, mouseX, mouseY);
			else if (button == 1)
				Kore::Mouse::the()->_press(0, 1, mouseX, mouseY);
		}
		else {
			if (button == 0)
				Kore::Mouse::the()->_release(0, 0, mouseX, mouseY);
			else if (button == 1)
				Kore::Mouse::the()->_release(0, 1, mouseX, mouseY);
		}
	}

	void onMouseMove(int x, int y) {
		mouseX = x;
		mouseY = y;
		Kore::Mouse::the()->_move(0, x, y);
	}
}

using namespace Kore;

namespace {
	int w, h;
}

int Kore::System::initWindow(WindowOptions options) {
	w = options.width;
	h = options.height;
	/*glutInit(&argc, argv);
	glutInitWindowSize(Kore::Application::the()->width(), Kore::Application::the()->height());
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Kore");
	glutIdleFunc(idlefunc);
	//glutReshapeFunc(reshapefunc);
	glutDisplayFunc(drawfunc);
	glutSpecialFunc(specialfunc);*/

	glfwInit();
	glfwOpenWindow(Kore::System::windowWidth(), Kore::System::windowHeight(), 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle(Kore::System::name());

	glfwSetKeyCallback(onKeyPressed);
	// glfwSetCharCallback(onCharPressed);
	// glfwSetWindowCloseCallback(onClose);
	// glfwSetWindowSizeCallback(onResize);
	// glfwSetWindowRefreshCallback(onRefresh);
	// glfwSetMouseWheelCallback(onMouseWheel);
	glfwSetMousePosCallback(onMouseMove);
	glfwSetMouseButtonCallback(onMouseClick);

	return nullptr;
}

void Kore::System::makeCurrent(int contextId) {}

void Kore::System::setup() {}

void Kore::System::clearCurrent() {}

int Kore::System::currentDevice() {
	return 0;
}

bool System::handleMessages() {
	return true;
}

void Kore::System::swapBuffers(int window) {}

void Kore::System::destroyWindow(int window) {}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {}

void Kore::System::setTitle(const char* title) {}

void Kore::System::setKeepScreenOn(bool on) {}

void Kore::System::showWindow() {}

double Kore::System::frequency() {
	return 1000.0;
}

int Kore::System::windowWidth(int window) {
	return w;
}

int Kore::System::windowHeight(int window) {
	return h;
}

int Kore::System::desktopWidth() {
	return w;
}

int Kore::System::desktopHeight() {
	return h;
}

Kore::System::ticks Kore::System::timestamp() {
	return static_cast<Kore::System::ticks>(glfwGetTime() * 1000.0);
}

double Kore::System::time() {
	// printf("Time: %f\n", glfwGetTime());
	return glfwGetTime();
}

extern int kore(int argc, char** argv);

int main(int argc, char** argv) {
	::argc = argc;
	::argv = argv;
	kore(argc, argv);
	// System::createWindow();
	// glutMainLoop();
	emscripten_set_main_loop(drawfunc, 0, 1);
}
