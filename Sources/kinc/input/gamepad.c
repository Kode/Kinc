#include "pch.h"

#include "gamepad.h"

#include <memory.h>

void (*Kinc_Gamepad_AxisCallback)(int /*gamepad*/, int /*axis*/, float /*value*/) = NULL;
void (*Kinc_Gamepad_ButtonCallback)(int /*gamepad*/, int /*button*/, float /*value*/) = NULL;

void Kinc_Internal_Gamepad_TriggerAxis(int gamepad, int axis, float value) {
	if (Kinc_Gamepad_AxisCallback != NULL) {
		Kinc_Gamepad_AxisCallback(gamepad, axis, value);
	}
}

void Kinc_Internal_Gamepad_TriggerButton(int gamepad, int button, float value) {
	if (Kinc_Gamepad_ButtonCallback != NULL) {
		Kinc_Gamepad_ButtonCallback(gamepad, button, value);
	}
}
