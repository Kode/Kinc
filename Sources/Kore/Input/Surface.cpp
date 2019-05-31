#include "pch.h"

#include "Surface.h"

#include <kinc/input/surface.h>

using namespace Kore;

namespace {
	Surface surface;
	bool initialized = false;

	void touchStart(int index, int x, int y) {
		if (surface.TouchStart != nullptr) {
			surface.TouchStart(index, x, y);
		}
	}

	void move(int index, int x, int y) {
		if (surface.Move != nullptr) {
			surface.Move(index, x, y);
		}
	}

	void touchEnd(int index, int x, int y) {
		if (surface.TouchEnd != nullptr) {
			surface.TouchEnd(index, x, y);
		}
	}
}

Surface* Surface::the() {
	if (!initialized) {
		Kinc_Surface_TouchStartCallback = touchStart;
		Kinc_Surface_MoveCallback = move;
		Kinc_Surface_TouchEndCallback = touchEnd;
		initialized = true;
	}
	return &surface;
}

Surface::Surface() : TouchStart(nullptr), Move(nullptr), TouchEnd(nullptr) {}
