#include "pch.h"

#include <kinc/input/mouse.h>

void kinc_internal_mouse_lock(int window) {

}

void kinc_internal_mouse_unlock(int window) {

}

bool kinc_mouse_can_lock(int window) {
	return false;
}

void kinc_mouse_show() {

}

void kinc_mouse_hide() {

}

void kinc_mouse_set_position(int window, int x, int y) {

}

void kinc_mouse_get_position(int window, int* x, int* y) {
	*x = 0;
	*y = 0;
}

void kinc_mouse_set_cursor(int cursor_index) {

}
