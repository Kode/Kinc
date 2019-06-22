#include "pch.h"

#include "pen.h"

#include <memory.h>

void (*kinc_pen_press_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
void (*kinc_pen_move_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
void (*kinc_pen_release_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;

void kinc_internal_pen_trigger_press(int window, int x, int y, float pressure) {
	if (kinc_pen_press_callback != NULL) {
		kinc_pen_press_callback(window, x, y, pressure);
	}
}

void kinc_internal_pen_trigger_move(int window, int x, int y, float pressure) {
	if (kinc_pen_move_callback != NULL) {
		kinc_pen_move_callback(window, x, y, pressure);
	}
}

void kinc_internal_pen_trigger_release(int window, int x, int y, float pressure) {
	if (kinc_pen_release_callback != NULL) {
		kinc_pen_release_callback(window, x, y, pressure);
	}
}
