#include "pch.h"
#include "Gamepad.h"

using namespace Kore;

namespace {
	Gamepad pads[12];
}

Gamepad* Gamepad::get(int num) {
	return &pads[num];
}

void Gamepad::_axis(int axis, float value) {
	if (Axis != nullptr) {
		Axis(axis, value);
	}
}

void Gamepad::_button(int button, float value) {
	if (Button != nullptr) {
		Button(button, value);
	}
}
