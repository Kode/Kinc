#include "gamepad.h"

#include <memory.h>

void (*kinc_gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/) = NULL;
void (*kinc_gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/) = NULL;

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value) {
	if (kinc_gamepad_axis_callback != NULL) {
		kinc_gamepad_axis_callback(gamepad, axis, value);
	}
}

void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value) {
	if (kinc_gamepad_button_callback != NULL) {
		kinc_gamepad_button_callback(gamepad, button, value);
	}
}
