#include "pch.h"

#include "Gamepad.h"

#include <Kinc/Input/Gamepad.h>

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
		Kinc_Gamepad_AxisCallback = axisCallback;
		Kinc_Gamepad_ButtonCallback = buttonCallback;
	}
	if (!padInitialized[num]) {
		pads[num].vendor = Kinc_Gamepad_Vendor(num);
		pads[num].productName = Kinc_Gamepad_ProductName(num);
		padInitialized[num] = true;
	}
	return &pads[num];
}
