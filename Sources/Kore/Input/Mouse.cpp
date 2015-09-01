#include "pch.h"
#include "Mouse.h"
#include <Kore/Application.h>
#include <Kore/System.h>

using namespace Kore;

namespace {
	Mouse mouse;
}

Mouse* Mouse::the() {
	return &mouse;
}

void Mouse::_move(int x, int y) {
	if (Move != nullptr) {
		Move(x, y);
	}
}

void Mouse::_press(int button, int x, int y) {
	if (Press != nullptr) {
		Press(button, x, y);
	}
}

void Mouse::_release(int button, int x, int y) {
	if (Release != nullptr) {
		Release(button, x, y);
	}
}

void Mouse::_scroll(int delta) {
	if (Scroll != nullptr) {
		Scroll(delta);
	}
}
