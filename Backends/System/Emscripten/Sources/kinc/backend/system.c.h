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
	if (!initialized)
		return;
	kinc_internal_update_callback();
	kinc_a2_update();
#ifdef KORE_OPENGL
	glfwSwapBuffers();
#endif
}

#define KEY_DOWN(GLFW_KEYCODE, KINC_KEY)                                                                                                                       \
	case GLFW_KEYCODE:                                                                                                                                         \
		kinc_internal_keyboard_trigger_key_down(KINC_KEY);                                                                                                     \
		break;

#define KEY_UP(GLFW_KEYCODE, KINC_KEY)                                                                                                                         \
	case GLFW_KEYCODE:                                                                                                                                         \
		kinc_internal_keyboard_trigger_key_up(KINC_KEY);                                                                                                       \
		break;

#ifdef KORE_OPENGL
// glfw mappings as state here: https://www.glfw.org/docs/3.3/group__keys.html
static void onKeyPressed(int key, int action) {
	if (action == GLFW_PRESS) {
		switch (key) {
			KEY_DOWN(32, KINC_KEY_SPACE)
			KEY_DOWN(39, KINC_KEY_QUOTE)
			KEY_DOWN(44, KINC_KEY_COMMA)
			KEY_DOWN(45, KINC_KEY_SUBTRACT)
			KEY_DOWN(46, KINC_KEY_PERIOD)
			KEY_DOWN(47, KINC_KEY_SLASH)
			KEY_DOWN(48, KINC_KEY_0)
			KEY_DOWN(49, KINC_KEY_1)
			KEY_DOWN(50, KINC_KEY_2)
			KEY_DOWN(51, KINC_KEY_3)
			KEY_DOWN(52, KINC_KEY_4)
			KEY_DOWN(53, KINC_KEY_5)
			KEY_DOWN(54, KINC_KEY_6)
			KEY_DOWN(55, KINC_KEY_7)
			KEY_DOWN(56, KINC_KEY_8)
			KEY_DOWN(57, KINC_KEY_9)
			KEY_DOWN(59, KINC_KEY_SEMICOLON)
			KEY_DOWN(61, KINC_KEY_EQUALS)
			KEY_DOWN(65, KINC_KEY_A)
			KEY_DOWN(66, KINC_KEY_B)
			KEY_DOWN(67, KINC_KEY_C)
			KEY_DOWN(68, KINC_KEY_D)
			KEY_DOWN(69, KINC_KEY_E)
			KEY_DOWN(70, KINC_KEY_F)
			KEY_DOWN(71, KINC_KEY_G)
			KEY_DOWN(72, KINC_KEY_H)
			KEY_DOWN(73, KINC_KEY_I)
			KEY_DOWN(74, KINC_KEY_J)
			KEY_DOWN(75, KINC_KEY_K)
			KEY_DOWN(76, KINC_KEY_L)
			KEY_DOWN(77, KINC_KEY_M)
			KEY_DOWN(78, KINC_KEY_N)
			KEY_DOWN(79, KINC_KEY_O)
			KEY_DOWN(80, KINC_KEY_P)
			KEY_DOWN(81, KINC_KEY_Q)
			KEY_DOWN(82, KINC_KEY_R)
			KEY_DOWN(83, KINC_KEY_S)
			KEY_DOWN(84, KINC_KEY_T)
			KEY_DOWN(85, KINC_KEY_U)
			KEY_DOWN(86, KINC_KEY_V)
			KEY_DOWN(87, KINC_KEY_W)
			KEY_DOWN(88, KINC_KEY_X)
			KEY_DOWN(89, KINC_KEY_Y)
			KEY_DOWN(90, KINC_KEY_Z)
			KEY_DOWN(92, KINC_KEY_BACK_SLASH)
			KEY_DOWN(256, KINC_KEY_ESCAPE)
			KEY_DOWN(257, KINC_KEY_RETURN)
			KEY_DOWN(258, KINC_KEY_TAB)
			KEY_DOWN(259, KINC_KEY_BACKSPACE)
			KEY_DOWN(260, KINC_KEY_INSERT)
			KEY_DOWN(261, KINC_KEY_DELETE)
			KEY_DOWN(262, KINC_KEY_RIGHT)
			KEY_DOWN(263, KINC_KEY_LEFT)
			KEY_DOWN(264, KINC_KEY_DOWN)
			KEY_DOWN(265, KINC_KEY_UP)
			KEY_DOWN(268, KINC_KEY_HOME)
			KEY_DOWN(269, KINC_KEY_END)
			KEY_DOWN(284, KINC_KEY_PAUSE)
			KEY_DOWN(290, KINC_KEY_F1)
			KEY_DOWN(291, KINC_KEY_F2)
			KEY_DOWN(292, KINC_KEY_F3)
			KEY_DOWN(293, KINC_KEY_F4)
			KEY_DOWN(294, KINC_KEY_F5)
			KEY_DOWN(295, KINC_KEY_F6)
			KEY_DOWN(296, KINC_KEY_F7)
			KEY_DOWN(297, KINC_KEY_F8)
			KEY_DOWN(298, KINC_KEY_F9)
			KEY_DOWN(299, KINC_KEY_F10)
			KEY_DOWN(300, KINC_KEY_F11)
			KEY_DOWN(301, KINC_KEY_F12)
			KEY_DOWN(302, KINC_KEY_F13)
			KEY_DOWN(303, KINC_KEY_F14)
			KEY_DOWN(304, KINC_KEY_F15)
			KEY_DOWN(305, KINC_KEY_F16)
			KEY_DOWN(306, KINC_KEY_F17)
			KEY_DOWN(307, KINC_KEY_F18)
			KEY_DOWN(308, KINC_KEY_F19)
			KEY_DOWN(309, KINC_KEY_F20)
			KEY_DOWN(310, KINC_KEY_F21)
			KEY_DOWN(311, KINC_KEY_F22)
			KEY_DOWN(312, KINC_KEY_F23)
			KEY_DOWN(313, KINC_KEY_F24)
			KEY_DOWN(348, KINC_KEY_CONTEXT_MENU)
		}
	}
	else {
		switch (key) {
			KEY_UP(32, KINC_KEY_SPACE)
			KEY_UP(39, KINC_KEY_QUOTE)
			KEY_UP(44, KINC_KEY_COMMA)
			KEY_UP(45, KINC_KEY_SUBTRACT)
			KEY_UP(46, KINC_KEY_PERIOD)
			KEY_UP(47, KINC_KEY_SLASH)
			KEY_UP(48, KINC_KEY_0)
			KEY_UP(49, KINC_KEY_1)
			KEY_UP(50, KINC_KEY_2)
			KEY_UP(51, KINC_KEY_3)
			KEY_UP(52, KINC_KEY_4)
			KEY_UP(53, KINC_KEY_5)
			KEY_UP(54, KINC_KEY_6)
			KEY_UP(55, KINC_KEY_7)
			KEY_UP(56, KINC_KEY_8)
			KEY_UP(57, KINC_KEY_9)
			KEY_UP(59, KINC_KEY_SEMICOLON)
			KEY_UP(61, KINC_KEY_EQUALS)
			KEY_UP(65, KINC_KEY_A)
			KEY_UP(66, KINC_KEY_B)
			KEY_UP(67, KINC_KEY_C)
			KEY_UP(68, KINC_KEY_D)
			KEY_UP(69, KINC_KEY_E)
			KEY_UP(70, KINC_KEY_F)
			KEY_UP(71, KINC_KEY_G)
			KEY_UP(72, KINC_KEY_H)
			KEY_UP(73, KINC_KEY_I)
			KEY_UP(74, KINC_KEY_J)
			KEY_UP(75, KINC_KEY_K)
			KEY_UP(76, KINC_KEY_L)
			KEY_UP(77, KINC_KEY_M)
			KEY_UP(78, KINC_KEY_N)
			KEY_UP(79, KINC_KEY_O)
			KEY_UP(80, KINC_KEY_P)
			KEY_UP(81, KINC_KEY_Q)
			KEY_UP(82, KINC_KEY_R)
			KEY_UP(83, KINC_KEY_S)
			KEY_UP(84, KINC_KEY_T)
			KEY_UP(85, KINC_KEY_U)
			KEY_UP(86, KINC_KEY_V)
			KEY_UP(87, KINC_KEY_W)
			KEY_UP(88, KINC_KEY_X)
			KEY_UP(89, KINC_KEY_Y)
			KEY_UP(90, KINC_KEY_Z)
			KEY_UP(92, KINC_KEY_BACK_SLASH)
			KEY_UP(256, KINC_KEY_ESCAPE)
			KEY_UP(257, KINC_KEY_RETURN)
			KEY_UP(258, KINC_KEY_TAB)
			KEY_UP(259, KINC_KEY_BACKSPACE)
			KEY_UP(260, KINC_KEY_INSERT)
			KEY_UP(261, KINC_KEY_DELETE)
			KEY_UP(262, KINC_KEY_RIGHT)
			KEY_UP(263, KINC_KEY_LEFT)
			KEY_UP(264, KINC_KEY_DOWN)
			KEY_UP(265, KINC_KEY_UP)
			KEY_UP(268, KINC_KEY_HOME)
			KEY_UP(269, KINC_KEY_END)
			KEY_UP(284, KINC_KEY_PAUSE)
			KEY_UP(290, KINC_KEY_F1)
			KEY_UP(291, KINC_KEY_F2)
			KEY_UP(292, KINC_KEY_F3)
			KEY_UP(293, KINC_KEY_F4)
			KEY_UP(294, KINC_KEY_F5)
			KEY_UP(295, KINC_KEY_F6)
			KEY_UP(296, KINC_KEY_F7)
			KEY_UP(297, KINC_KEY_F8)
			KEY_UP(298, KINC_KEY_F9)
			KEY_UP(299, KINC_KEY_F10)
			KEY_UP(300, KINC_KEY_F11)
			KEY_UP(301, KINC_KEY_F12)
			KEY_UP(302, KINC_KEY_F13)
			KEY_UP(303, KINC_KEY_F14)
			KEY_UP(304, KINC_KEY_F15)
			KEY_UP(305, KINC_KEY_F16)
			KEY_UP(306, KINC_KEY_F17)
			KEY_UP(307, KINC_KEY_F18)
			KEY_UP(308, KINC_KEY_F19)
			KEY_UP(309, KINC_KEY_F20)
			KEY_UP(310, KINC_KEY_F21)
			KEY_UP(311, KINC_KEY_F22)
			KEY_UP(312, KINC_KEY_F23)
			KEY_UP(313, KINC_KEY_F24)
			KEY_UP(348, KINC_KEY_CONTEXT_MENU)
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
	glfwOpenWindow(width, height, 8, 8, 8, 0, frame->depth_bits, frame->stencil_bits, GLFW_WINDOW);
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

int kinc_cpu_cores(void) {
	return 4;
}

int kinc_hardware_threads(void) {
	return 4;
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
