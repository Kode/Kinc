#include "pch.h"
#include "Keyboard.h"
#include "KeyEvent.h"

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

void Keyboard::keydown(KeyEvent e) {
	if (KeyDown != nullptr) KeyDown(&e);
}

void Keyboard::keyup(KeyEvent e) {
	if (KeyUp != nullptr) KeyUp(&e);
}
