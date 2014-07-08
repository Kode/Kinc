#include "pch.h"
#include "Surface.h"

using namespace Kore;

namespace {
	Surface surface;
}

Surface* Surface::the() {
	return &surface;
}

void Surface::_move(int index, int x, int y) {
	if (Move != nullptr) {
		Move(index, x, y);
	}
}

void Surface::_touchStart(int index, int x, int y) {
	if (TouchStart != nullptr) {
		TouchStart(index, x, y);
	}
}

void Surface::_touchEnd(int index, int x, int y) {
	if (TouchEnd != nullptr) {
		TouchEnd(index, x, y);
	}
}
