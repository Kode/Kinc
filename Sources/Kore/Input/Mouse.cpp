#include "pch.h"
#include "Mouse.h"

using namespace Kore;

namespace {
	Mouse mouse;
}

Mouse* Mouse::the() {
	return &mouse;
}

void Mouse::_move(MouseEvent event) {
	if (Move != nullptr) Move(event);
}

void Mouse::_pressLeft(MouseEvent event) {
	if (PressLeft != nullptr) PressLeft(event);
}

void Mouse::_pressRight(MouseEvent event) {
	if (PressRight != nullptr) PressRight(event);
}

void Mouse::_releaseLeft(MouseEvent event) {
	if (ReleaseLeft != nullptr) ReleaseLeft(event);
}

void Mouse::_releaseRight(MouseEvent event) {
	if (ReleaseRight != nullptr) ReleaseRight(event);
}
