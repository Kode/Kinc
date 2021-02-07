#include "pch.h"

#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>

int kinc_window_create(kinc_window_options_t* win, kinc_framebuffer_options_t* frame) {
	return 0;
}

void kinc_window_destroy(int window_index) {

}

int kinc_count_windows(void) {
	return 1;
}

void kinc_window_resize(int window_index, int width, int height) {

}

void kinc_window_move(int window_index, int x, int y) {

}

void kinc_window_change_mode(int window_index, kinc_window_mode_t mode) {

}

void kinc_window_change_features(int window_index, int features) {

}

extern "C" void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame);

void kinc_window_change_framebuffer(int window_index, kinc_framebuffer_options_t* frame) {
	kinc_internal_change_framebuffer(0, frame);
}

int kinc_window_x(int window_index) {
	return 0;
}

int kinc_window_y(int window_index) {
	return 0;
}

int kinc_window_display(int window_index) {
	return 0;
}

kinc_window_mode_t kinc_window_get_mode(int window_index) {
	return KINC_WINDOW_MODE_WINDOW;
}

void kinc_window_show(int window_index) {

}

void kinc_window_hide(int window_index) {

}

void kinc_window_set_title(int window_index, const char* title) {

}

void kinc_window_set_resize_callback(int window_index, void (*callback)(int x, int y, void* data), void* data) {

}

void kinc_window_set_ppi_changed_callback(int window_index, void (*callback)(int ppi, void* data), void* data) {

}
