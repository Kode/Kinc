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

	void drawfunc() {
		Kore::System::_callback();
		Kore::Audio2::update();
		// glutSwapBuffers();
		glfwSwapBuffers();
	}

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
			case 256:
				Kore::Keyboard::the()->_keydown(Kore::KeyEscape);
				break;
			case 82:
				Kore::Keyboard::the()->_keydown(Kore::KeyR);
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
			case 256:
				Kore::Keyboard::the()->_keyup(Kore::KeyEscape);
				break;
			case 82:
				Kore::Keyboard::the()->_keyup(Kore::KeyR);
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


Kore::Window* Kore::System::init(const char* name, int width, int height, WindowOptions* win, FramebufferOptions* frame) {
	//**Display::enumerate();
	printf("Init\n");
	System::_init(name, width, height, &win, &frame);
	glfwInit();
	glfwOpenWindow(width,  height, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle(name);
	glfwSetKeyCallback(onKeyPressed);
	glfwSetMousePosCallback(onMouseMove);
	glfwSetMouseButtonCallback(onMouseClick);
	Kore::Window* window = Kore::Window::get(0);
	window->_data.width = width;
	window->_data.height = height;
	return window;
}

bool System::handleMessages() {
	return true;
}

void Kore::System::setKeepScreenOn(bool on) {}

double Kore::System::frequency() {
	return 1000.0;
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
