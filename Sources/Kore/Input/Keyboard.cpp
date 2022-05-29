#include "Keyboard.h"

using namespace Kore;

namespace {
	Keyboard keyboard;
	bool initialized = false;

	void down(int code) {
		if (keyboard.KeyDown != nullptr) {
			keyboard.KeyDown((KeyCode)code);
		}
	}

	void up(int code) {
		if (keyboard.KeyUp != nullptr) {
			keyboard.KeyUp((KeyCode)code);
		}
	}

	void press(unsigned character) {
		if (keyboard.KeyPress != nullptr) {
			keyboard.KeyPress((wchar_t)character);
		}
	}
}

Keyboard *Keyboard::the() {
	if (!initialized) {
		kinc_keyboard_set_key_down_callback(down);
		kinc_keyboard_set_key_up_callback(up);
		kinc_keyboard_set_key_press_callback(press);
		initialized = true;
	}
	return &keyboard;
}

void Keyboard::clear() {
	KeyDown = nullptr;
	KeyUp = nullptr;
	KeyPress = nullptr;
}

Keyboard::Keyboard() : KeyDown(nullptr), KeyUp(nullptr), KeyPress(nullptr) {}
