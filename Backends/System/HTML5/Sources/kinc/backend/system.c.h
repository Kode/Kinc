#ifdef KORE_OPENGL
#include <GL/glfw.h>
#endif

#include <emscripten/emscripten.h>
#include <kinc/audio2/audio.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <stdio.h>
#include <stdlib.h>

static int html5_argc;
static char **html5_argv;
static bool initialized = false;

static void drawfunc() {
	if (!initialized) return;
	kinc_internal_update_callback();
	kinc_a2_update();
#ifdef KORE_OPENGL
	glfwSwapBuffers();
#endif
}

#ifdef KORE_OPENGL
static void onKeyPressed(int key, int action) {
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

static int mouseX = 0;
static int mouseY = 0;

static void onMouseClick(int button, int action) {
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

static void onMouseMove(int x, int y) {
	mouseX = x;
	mouseY = y;
	kinc_internal_mouse_trigger_move(0, x, y);
}
#endif

static int with, height;

extern int kinc_internal_window_width;
extern int kinc_internal_window_height;

int kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}
	win->width = width;
	win->height = height;

#ifdef KORE_OPENGL
	glfwInit();
	glfwOpenWindow(width, height, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle(name);
	glfwSetKeyCallback(onKeyPressed);
	glfwSetMousePosCallback(onMouseMove);
	glfwSetMouseButtonCallback(onMouseClick);
#endif
	kinc_internal_window_width = width;
	kinc_internal_window_height = height;
	kinc_g4_internal_init();
	kinc_g4_internal_init_window(0, frame->depth_bits, frame->stencil_bits, true);
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
#ifdef KORE_OPENGL
	return (kinc_ticks_t)(glfwGetTime() * 1000.0);
#else
	return (kinc_ticks_t)(0.0);
#endif
}

double kinc_time(void) {
#ifdef KORE_OPENGL
	return glfwGetTime();
#else
	return 0.0;
#endif
}

extern int kickstart(int argc, char **argv);

#ifdef KORE_WEBGPU
EMSCRIPTEN_KEEPALIVE void kinc_internal_webgpu_initialized() {
	kickstart(html5_argc, html5_argv);
	initialized = true;
}
#endif

int main(int argc, char **argv) {
	html5_argc = argc;
	html5_argv = argv;
#ifdef KORE_WEBGPU
	char *code = "(async () => {\
		const adapter = await navigator.gpu.requestAdapter();\
		const device = await adapter.requestDevice();\
		Module.preinitializedWebGPUDevice = device;\
		_kinc_internal_webgpu_initialized();\
	})();";
	emscripten_run_script(code);
#else
	kickstart(argc, argv);
	initialized = true;
#endif
	emscripten_set_main_loop(drawfunc, 0, 1);
}
