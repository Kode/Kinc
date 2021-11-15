#include "gamepad.h"

#include <memory.h>

static void (*gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/) = NULL;
static void (*gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/) = NULL;

void kinc_set_gamepad_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/)) {
	gamepad_axis_callback = value;
}
void kinc_set_gamepad_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/)) {
	gamepad_button_callback = value;
}
void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value) {
	if (gamepad_axis_callback != NULL) {
		gamepad_axis_callback(gamepad, axis, value);
	}
}

void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value) {
	if (gamepad_button_callback != NULL) {
		gamepad_button_callback(gamepad, button, value);
	}
}
