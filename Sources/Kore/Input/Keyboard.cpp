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
}

void Keyboard::_keydown(KeyCode code, wchar_t character) {
	if (KeyDown != nullptr) KeyDown(code, character);
}

void Keyboard::_keyup(KeyCode code, wchar_t character) {
	if (KeyUp != nullptr) KeyUp(code, character);
}
