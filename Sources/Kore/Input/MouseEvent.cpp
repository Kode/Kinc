#include "pch.h"
#include "MouseEvent.h"

using namespace Kore;

MouseEvent::MouseEvent() : ignored(true) {

}

MouseEvent::MouseEvent(int x, int y) : myGlobalX(x), myGlobalY(y), currentX(x), currentY(y), ignored(false) {

}

void MouseEvent::ignore() {
	ignored = true;
}

void MouseEvent::notIgnore() {
	ignored = false;
}

int MouseEvent::x() const {
	return currentX;
}

int MouseEvent::y() const {
	return currentY;
}

int MouseEvent::globalX() const {
	return myGlobalX;
}

int MouseEvent::globalY() const {
	return myGlobalY;
}

bool MouseEvent::isIgnored() const {
	return ignored;
}

void MouseEvent::translate(int xdif, int ydif) {
	currentX += xdif;
	currentY += ydif;
}
