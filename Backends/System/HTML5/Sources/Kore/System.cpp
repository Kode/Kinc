#include "pch.h"

#include <GL/glfw.h>
#include <kinc/audio2/audio.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <Kore/ogl.h>
#include <cstring>
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <stdlib.h>

namespace {
	int argc;
	char** argv;

	void drawfunc() {
		kinc_internal_update_callback();
		kinc_a2_update();
		// glutSwapBuffers();
		glfwSwapBuffers();
	}

	void onKeyPressed(int key, int action) {
		if (action == GLFW_PRESS) {
			switch (key) {
			case 87:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_W);
				break;
			case 65:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_A);
				break;
			case 83:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_S);
				break;
			case 68:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_D);
				break;
			case 32:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SPACE);
				break;
			case 262:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_RIGHT);
				break;
			case 263:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_LEFT);
				break;
			case 265:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_UP);
				break;
			case 264:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_DOWN);
				break;
			case 256:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_ESCAPE);
				break;
			case 82:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_R);
				break;
			}
		}
		else {
			switch (key) {
			case 87:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_W);
				break;
			case 65:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_A);
				break;
			case 83:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_S);
				break;
			case 68:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_D);
				break;
			case 32:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SPACE);
				break;
			case 262:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_RIGHT);
				break;
			case 263:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_LEFT);
				break;
			case 265:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_UP);
				break;
			case 264:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_DOWN);
				break;
			case 256:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_ESCAPE);
				break;
			case 82:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_R);
				break;
			}
		}
	}

	int mouseX = 0;
	int mouseY = 0;

	void onMouseClick(int button, int action) {
		if (action == GLFW_PRESS) {
			if (button == 0) {
				kinc_internal_mouse_trigger_press(0, 0, mouseX, mouseY);
			}
			else if (button == 1) {
				kinc_internal_mouse_trigger_press(0, 1, mouseX, mouseY);
			}
		}
		else {
			if (button == 0) {
				kinc_internal_mouse_trigger_release(0, 0, mouseX, mouseY);
			}
			else if (button == 1) {
				kinc_internal_mouse_trigger_release(0, 1, mouseX, mouseY);
			}
		}
	}

	void onMouseMove(int x, int y) {
		mouseX = x;
		mouseY = y;
		kinc_internal_mouse_trigger_move(0, x, y);
	}
}

using namespace Kore;

namespace {
	int w, h;
}

extern int kinc_internal_window_width;
extern int kinc_internal_window_height;

int kinc_init(const char* name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	//**Display::enumerate();
	printf("Init\n");

	kinc_window_options_t defaultWin;
    if (win == NULL) {
        kinc_internal_init_window_options(&defaultWin);
        win = &defaultWin;
    }
    kinc_framebuffer_options_t defaultFrame;
    if (frame == NULL) {
        kinc_internal_init_framebuffer_options(&defaultFrame);
        frame = &defaultFrame;
    }
    win->width = width;
    win->height = height;

	glfwInit();
	glfwOpenWindow(width,  height, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle(name);
	glfwSetKeyCallback(onKeyPressed);
	glfwSetMousePosCallback(onMouseMove);
	glfwSetMouseButtonCallback(onMouseClick);
	kinc_internal_window_width = width;
	kinc_internal_window_height = height;
	return 0;
}

bool kinc_internal_handle_messages() {
	return true;
}

void kinc_set_keep_screen_on(bool on) {}

double kinc_frequency(void) {
	return 1000.0;
}

kinc_ticks_t kinc_timestamp(void) {
	return (kinc_ticks_t)(glfwGetTime() * 1000.0);
}

double kinc_time(void) {
	// printf("Time: %f\n", glfwGetTime());
	return glfwGetTime();
}

extern int kickstart(int argc, char** argv);

int main(int argc, char** argv) {
	::argc = argc;
	::argv = argv;
	kickstart(argc, argv);
	// System::createWindow();
	// glutMainLoop();
	emscripten_set_main_loop(drawfunc, 0, 1);
}
