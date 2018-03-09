#include "pch.h"

#include "Pen.h"

#include <Kore/Log.h>
#include <Kore/System.h>

using namespace Kore;

namespace {
	Pen pen;
}

Pen* Pen::the() {
	return &pen;
}

Pen::Pen() {}

void Pen::_move(int windowId, int x, int y, float pressure) {
	if (Move != nullptr) {
		Move(windowId, x, y, pressure);
	}
}

void Pen::_press(int windowId, int x, int y, float pressure) {
	if (Press != nullptr) {
		Press(windowId, x, y, pressure);
	}
}

void Pen::_release(int windowId, int x, int y, float pressure) {
	if (Release != nullptr) {
		Release(windowId, x, y, pressure);
	}
}
