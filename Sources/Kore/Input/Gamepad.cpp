#include "pch.h"

#include "Gamepad.h"

#include <kinc/input/gamepad.h>

using namespace Kore;

namespace {
	const int maxGamepads = 12;
	Gamepad pads[maxGamepads];
	bool padInitialized[maxGamepads] = {0};
	bool callbacksInitialized = false;

	void axisCallback(int gamepad, int axis, float value) {
		if (gamepad < maxGamepads && padInitialized[gamepad] && pads[gamepad].Axis != nullptr) {
			pads[gamepad].Axis(axis, value);
		}
	}

	void buttonCallback(int gamepad, int button, float value) {
		if (gamepad < maxGamepads && padInitialized[gamepad] && pads[gamepad].Button != nullptr) {
			pads[gamepad].Button(button, value);
		}
	}
}

Gamepad* Gamepad::get(int num) {
	if (num >= maxGamepads) {
		return nullptr;
	}
	if (!callbacksInitialized) {
		kinc_gamepad_axis_callback = axisCallback;
		kinc_gamepad_button_callback = buttonCallback;
	}
	if (!padInitialized[num]) {
		pads[num].vendor = kinc_gamepad_vendor(num);
		pads[num].productName = kinc_gamepad_product_name(num);
		pads[num].num = num;
		padInitialized[num] = true;
	}
	return &pads[num];
}

bool Gamepad::connected() {
	return kinc_gamepad_connected(num);
}
