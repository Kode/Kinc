#include "mouse.h"

#include <kinc/window.h>

#include <memory.h>

static void (*mouse_press_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/) = NULL;
static void (*mouse_release_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/) = NULL;
static void (*mouse_move_callback)(int /*window*/, int /*x*/, int /*y*/, int /*movementX*/, int /*movementY*/) = NULL;
static void (*mouse_scroll_callback)(int /*window*/, int /*delta*/) = NULL;
static void (*mouse_enter_window_callback)(int /*window*/) = NULL;
static void (*mouse_leave_window_callback)(int /*window*/) = NULL;

void kinc_mouse_set_press_callback(void (*value)(int /*window*/, int /*button*/, int /*x*/, int /*y*/)) {
	mouse_press_callback = value;
}

void kinc_mouse_set_release_callback(void (*value)(int /*window*/, int /*button*/, int /*x*/, int /*y*/)) {
	mouse_release_callback = value;
}

void kinc_mouse_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/)) {
	mouse_move_callback = value;
}

void kinc_mouse_set_scroll_callback(void (*value)(int /*window*/, int /*delta*/)) {
	mouse_scroll_callback = value;
}

void kinc_mouse_set_enter_window_callback(void (*value)(int /*window*/)) {
	mouse_enter_window_callback = value;
}

void kinc_mouse_set_leave_window_callback(void (*value)(int /*window*/)) {
	mouse_leave_window_callback = value;
}

void kinc_internal_mouse_trigger_release(int window, int button, int x, int y) {
	if (mouse_release_callback != NULL) {
		mouse_release_callback(window, button, x, y);
	}
}

void kinc_internal_mouse_trigger_scroll(int window, int delta) {
	if (mouse_scroll_callback != NULL) {
		mouse_scroll_callback(window, delta);
	}
}

void kinc_internal_mouse_trigger_enter_window(int window) {
	if (mouse_enter_window_callback != NULL) {
		mouse_enter_window_callback(window);
	}
}

void kinc_internal_mouse_trigger_leave_window(int window) {
	if (mouse_leave_window_callback != NULL) {
		mouse_leave_window_callback(window);
	}
}

void kinc_internal_mouse_window_activated(int window) {
	if (kinc_mouse_is_locked()) {
		kinc_internal_mouse_lock(window);
	}
}
void kinc_internal_mouse_window_deactivated(int window) {
	if (kinc_mouse_is_locked()) {
		kinc_internal_mouse_unlock();
	}
}

// TODO: handle state per window
static bool moved = false;
static bool locked = false;
static int preLockWindow = 0;
static int preLockX = 0;
static int preLockY = 0;
static int centerX = 0;
static int centerY = 0;
static int lastX = 0;
static int lastY = 0;

void kinc_internal_mouse_trigger_press(int window, int button, int x, int y) {
	lastX = x;
	lastY = y;
	if (mouse_press_callback != NULL) {
		mouse_press_callback(window, button, x, y);
	}
}

void kinc_internal_mouse_trigger_move(int window, int x, int y) {
	int movementX = 0;
	int movementY = 0;
	if (kinc_mouse_is_locked()) {
		movementX = x - centerX;
		movementY = y - centerY;
		if (movementX != 0 || movementY != 0) {
			kinc_mouse_set_position(window, centerX, centerY);
			x = centerX;
			y = centerY;
		}
	}
	else if (moved) {
		movementX = x - lastX;
		movementY = y - lastY;
	}
	moved = true;

	lastX = x;
	lastY = y;
	if (mouse_move_callback != NULL && (movementX != 0 || movementY != 0)) {
		mouse_move_callback(window, x, y, movementX, movementY);
	}
}

bool kinc_mouse_is_locked(void) {
	return locked;
}

void kinc_mouse_lock(int window) {
	if (!kinc_mouse_can_lock()) {
		return;
	}
	locked = true;
	kinc_internal_mouse_lock(window);
	kinc_mouse_get_position(window, &preLockX, &preLockY);
	centerX = kinc_window_width(window) / 2;
	centerY = kinc_window_height(window) / 2;
	kinc_mouse_set_position(window, centerX, centerY);
}

void kinc_mouse_unlock(void) {
	if (!kinc_mouse_can_lock()) {
		return;
	}
	moved = false;
	locked = false;
	kinc_internal_mouse_unlock();
	kinc_mouse_set_position(preLockWindow, preLockX, preLockY);
}
