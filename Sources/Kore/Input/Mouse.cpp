#include "pch.h"
#include "Mouse.h"
#include <Kore/Application.h>
#include <Kore/System.h>

using namespace Kore;

namespace {
	Mouse mouse;
	
	void adjust(MouseEvent& event) {
		event.multiply(static_cast<float>(Kore::Application::the()->width()) / static_cast<float>(Kore::System::screenWidth()),
					   static_cast<float>(Kore::Application::the()->height()) / static_cast<float>(Kore::System::screenHeight()));
	}
}

Mouse* Mouse::the() {
	return &mouse;
}

void Mouse::_move(MouseEvent event) {
	if (Move != nullptr) {
		adjust(event);
		Move(event);
	}
}

void Mouse::_pressLeft(MouseEvent event) {
	if (PressLeft != nullptr) {
		adjust(event);
		PressLeft(event);
	}
}

void Mouse::_pressRight(MouseEvent event) {
	if (PressRight != nullptr) {
		adjust(event);
		PressRight(event);
	}
}

void Mouse::_releaseLeft(MouseEvent event) {
	if (ReleaseLeft != nullptr) {
		adjust(event);
		ReleaseLeft(event);
	}
}

void Mouse::_releaseRight(MouseEvent event) {
	if (ReleaseRight != nullptr) {
		adjust(event);
		ReleaseRight(event);
	}
}
