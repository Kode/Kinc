#include "surface.h"

#include <memory.h>

static void (*surface_touch_start_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
static void (*surface_move_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
static void (*surface_touch_end_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;

void kinc_set_surface_touch_start_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/)) {
	surface_touch_start_callback = value;
}

void kinc_set_surface_move_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/)) {
	surface_move_callback = value;
}

void kinc_set_surface_touch_end_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/)) {
	surface_touch_end_callback = value;
}

void kinc_internal_surface_trigger_touch_start(int index, int x, int y) {
	if (surface_touch_start_callback != NULL) {
		surface_touch_start_callback(index, x, y);
	}
}

void kinc_internal_surface_trigger_move(int index, int x, int y) {
	if (surface_move_callback != NULL) {
		surface_move_callback(index, x, y);
	}
}

void kinc_internal_surface_trigger_touch_end(int index, int x, int y) {
	if (surface_touch_end_callback != NULL) {
		surface_touch_end_callback(index, x, y);
	}
}
