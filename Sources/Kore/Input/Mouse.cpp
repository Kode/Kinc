#include "pch.h"
#include "Mouse.h"
#include <Kore/System.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	Mouse mouse;
}

Mouse* Mouse::the() {
	return &mouse;
}

Mouse::Mouse()
: lastX(0)
, lastY(0)
, lockX(0)
, lockY(0)
, centerX(0)
, centerY(0)
, locked(false)
, moved(false)
{}

void Mouse::_move(int windowId, int x, int y) {
	int movementX = 0;
	int movementY = 0;
	if (isLocked(windowId)){
		movementX = x - centerX;
		movementY = y - centerY;
		if (movementX != 0 || movementY != 0){
			setPosition(windowId, centerX, centerY);
			x = centerX;
			y = centerY;
		}
	}else if (moved){
		movementX = x - lastX;
		movementY = y - lastY;
	}
	moved = true;

	lastX = x;
	lastY = y;
	if (Move != nullptr && (movementX != 0 || movementY != 0)) {
		Move(windowId, x, y, movementX, movementY);
	}
}

void Mouse::_press(int windowId, int button, int x, int y) {
	if (Press != nullptr) {
		Press(windowId, button, x, y);
	}
}

void Mouse::_release(int windowId, int button, int x, int y) {
	if (Release != nullptr) {
		Release(windowId, button, x, y);
	}
}

void Mouse::_scroll(int windowId, int delta) {
	if (Scroll != nullptr) {
		Scroll(windowId, delta);
	}
}

void Mouse::_activated(int windowId, bool truth){
	if (isLocked(windowId)){
		_lock(windowId, truth);
	}
}

bool Mouse::isLocked(int windowId){
	return locked;
}

void Mouse::lock(int windowId){
	if (!canLock(windowId)){
		return;
	}
	locked = true;
	_lock(windowId, true);
	getPosition(windowId, lockX, lockY);
	centerX = Kore::System::windowHeight(windowId) / 2;
	centerY = Kore::System::windowHeight(windowId) / 2;
	setPosition(windowId, centerX, centerY);
}

void Mouse::unlock(int windowId){
	if (!canLock(windowId)){
		return;
	}
	moved = false;
	locked = false;
	_lock(windowId, false);
	setPosition(windowId, lockX, lockY);
}
