#include "pch.h"

#include "Pen.h"

#include <kinc/input/pen.h>

using namespace Kore;

namespace {
	Pen pen;
	bool initialized = false;

	void pressCallback(int window, int x, int y, float pressure) {
		if (pen.Press != nullptr) {
			pen.Press(window, x, y, pressure);
		}
	}

	void moveCallback(int window, int x, int y, float pressure) {
		if (pen.Move != nullptr) {
			pen.Move(window, x, y, pressure);
		}
	}

	void releaseCallback(int window, int x, int y, float pressure) {
		if (pen.Release != nullptr) {
			pen.Release(window, x, y, pressure);
		}
	}
}

Pen* Pen::the() {
	if (!initialized) {
		Kinc_Pen_PressCallback = pressCallback;
		Kinc_Pen_MoveCallback = moveCallback;
		Kinc_Pen_ReleaseCallback = releaseCallback;
		initialized = true;
	}
	return &pen;
}

Pen::Pen() : Move(nullptr), Press(nullptr), Release(nullptr) {}
