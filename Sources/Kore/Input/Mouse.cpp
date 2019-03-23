#include "pch.h"

#include "Mouse.h"

#include <Kinc/Input/Mouse.h>

using namespace Kore;

namespace {
	Mouse mouse;
	bool initialized = false;

	void move(int window, int x, int y, int movementX, int movementY) {
		if (mouse.Move != nullptr) {
			mouse.Move(window, x, y, movementX, movementY);
		}
	}

	void press(int window, int button, int x, int y) {
		if (mouse.Press != nullptr) {
			mouse.Press(window, button, x, y);
		}
	}

	void release(int window, int button, int x, int y) {
		if (mouse.Release != nullptr) {
			mouse.Release(window, button, x, y);
		}
	}

	void scroll(int window, int delta) {
		if (mouse.Scroll != nullptr) {
			mouse.Scroll(window, delta);
		}
	}

	void leave(int window) {
		if (mouse.Leave != nullptr) {
			mouse.Leave(window);
		}
	}
}

Mouse* Mouse::the() {
	if (!initialized) {
		Kinc_Mouse_MoveCallback = move;
		Kinc_Mouse_PressCallback = press;
		Kinc_Mouse_ReleaseCallback = release;
		Kinc_Mouse_ScrollCallback = scroll;
		Kinc_Mouse_LeaveWindowCallback = leave;
		initialized = true;
	}
	return &mouse;
}

Mouse::Mouse() : Move(nullptr), Press(nullptr), Release(nullptr), Scroll(nullptr), Leave(nullptr) {}

bool Mouse::canLock(int window) {
	return Kinc_Mouse_CanLock(window);
}

bool Mouse::isLocked(int window) {
	return Kinc_Mouse_IsLocked(window);
}

void Mouse::lock(int window) {
	Kinc_Mouse_Lock(window);
}

void Mouse::unlock(int window) {
	Kinc_Mouse_Unlock(window);
}

void Mouse::show(bool truth) {
	if (truth) {
		Kinc_Mouse_Show();
	}
	else {
		Kinc_Mouse_Hide();
	}
}

void Mouse::setPosition(int window, int x, int y) {
	Kinc_Mouse_SetPosition(window, x, y);
}

void Mouse::getPosition(int window, int &x, int &y) {
	Kinc_Mouse_GetPosition(window, &x, &y);
}
