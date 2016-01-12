#include "pch.h"
#include "Mouse.h"
#include <Kore/Application.h>
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

void Mouse::_move(int x, int y) {
	int movementX = 0;
	int movementY = 0;
	if (isLocked()){
		movementX = x - centerX;
		movementY = y - centerY;
		if (movementX != 0 || movementY != 0){
			setPosition(centerX, centerY);
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
		Move(x, y, movementX, movementY);
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

void Mouse::_activated(bool truth){
	if (isLocked()){
		_lock(truth);
	}
}



bool Mouse::isLocked(){
    log(Info, "mouse locked");
	return locked;
}

void Mouse::lock(){
	if (!canLock()){
		return;
	}
	locked = true;
	_lock(true);
	getPosition(lockX, lockY);
	centerX = Application::the()->width() / 2;
	centerY = Application::the()->height() / 2;
	setPosition(centerX, centerY);
}

void Mouse::unlock(){
	if (!canLock()){
		return;
	}
	moved = false;
	locked = false;
	_lock(false);
	setPosition(lockX, lockY);
}
