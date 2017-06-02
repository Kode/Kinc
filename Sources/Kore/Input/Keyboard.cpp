#include "pch.h"

#include "Keyboard.h"

using namespace Kore;

namespace {
	Keyboard keyboard;
}

Keyboard* Keyboard::the() {
	return &keyboard;
}

void Keyboard::clear() {
	KeyDown = nullptr;
	KeyUp = nullptr;
	KeyPress = nullptr;
}

void Keyboard::_keydown(KeyCode code) {
	if (KeyDown != nullptr) KeyDown(code);
}

void Keyboard::_keyup(KeyCode code) {
	if (KeyUp != nullptr) KeyUp(code);
}

void Keyboard::_keypress(wchar_t character) {
	if (KeyPress != nullptr) KeyPress(character);
}
