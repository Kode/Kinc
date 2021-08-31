#include "surface.h"

#include <memory.h>

void (*kinc_surface_touch_start_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
void (*kinc_surface_move_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
void (*kinc_surface_touch_end_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;

void kinc_internal_surface_trigger_touch_start(int index, int x, int y) {
	if (kinc_surface_touch_start_callback != NULL) {
		kinc_surface_touch_start_callback(index, x, y);
	}
}

void kinc_internal_surface_trigger_move(int index, int x, int y) {
	if (kinc_surface_move_callback != NULL) {
		kinc_surface_move_callback(index, x, y);
	}
}

void kinc_internal_surface_trigger_touch_end(int index, int x, int y) {
	if (kinc_surface_touch_end_callback != NULL) {
		kinc_surface_touch_end_callback(index, x, y);
	}
}
