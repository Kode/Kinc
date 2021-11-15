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

Surface *Surface::the() {
	if (!initialized) {
		kinc_surface_set_touch_start_callback(touchStart);
		kinc_surface_set_move_callback(move);
		kinc_surface_set_touch_end_callback(touchEnd);
		initialized = true;
	}
	return &surface;
}

Surface::Surface() : Move(nullptr), TouchStart(nullptr), TouchEnd(nullptr) {}
