#include "pch.h"

#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>
#include <Kore/Linux.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <string.h>

#include "WindowData.h"

#define MAXIMUM_WINDOWS 16
Kore::WindowData kinc_internal_windows[MAXIMUM_WINDOWS] = {0};

int kinc_count_windows(void) {
	return 1;
}

int kinc_window_x(int window_index) {
	return 0;
}

int kinc_window_y(int window_index) {
	return 0;
}

int kinc_window_width(int window_index) {
    return kinc_internal_windows[window_index].width;
}

int kinc_window_height(int window_index) {
    return kinc_internal_windows[window_index].height;
}

void kinc_window_resize(int window_index, int width, int height) {

}

void kinc_window_move(int window_index, int x, int y) {

}

void kinc_window_change_framebuffer(int window_index, kinc_framebuffer_options_t *frame) {
	//**kinc_g4_changeFramebuffer(0, frame);
}

void kinc_window_change_features(int window_index, int features) {

}

void Kore::Linux::fullscreen(XID window, bool value) {
#ifdef KORE_OPENGL
	Atom wm_state = XInternAtom(Kore::Linux::display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(Kore::Linux::display, "_NET_WM_STATE_FULLSCREEN", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = value ? 1 : 0;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XMapWindow(Kore::Linux::display, window);

	XSendEvent(Kore::Linux::display, DefaultRootWindow(Kore::Linux::display), False,
			   SubstructureRedirectMask | SubstructureNotifyMask, &xev);

	XFlush(Kore::Linux::display);
#endif
}

void kinc_window_change_mode(int window_index, kinc_window_mode_t mode) {
	if (mode == KINC_WINDOW_MODE_FULLSCREEN || mode == KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
		if (kinc_internal_windows[window_index].mode == KINC_WINDOW_MODE_FULLSCREEN || kinc_internal_windows[window_index].mode == KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
            kinc_internal_windows[window_index].mode = mode;
			return;
		}

		Kore::Linux::fullscreen(kinc_internal_windows[window_index].handle, true);
        kinc_internal_windows[window_index].mode = mode;
	}
	else {
		if (mode == kinc_internal_windows[window_index].mode) {
			return;
		}

		Kore::Linux::fullscreen(kinc_internal_windows[window_index].handle, false);
        kinc_internal_windows[window_index].mode = mode;
	}
}

int kinc_window_display(int window_index) {
	return 0;
}

void kinc_window_destroy(int window_index) {

}

void kinc_window_show(int window_index) {

}

void kinc_window_hide(int window_index) {

}

void kinc_window_set_title(int window_index, const char *title) {
#ifdef KORE_OPENGL
	XStoreName(Kore::Linux::display, kinc_internal_windows[window_index].handle, title);
#endif
}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	return 0;
}

void kinc_window_set_resize_callback(int window_index, void (*callback)(int x, int y, void *data), void *data) {

}

void kinc_window_set_ppi_changed_callback(int window_index, void (*callback)(int ppi, void *data), void *data) {

}

kinc_window_mode_t kinc_window_get_mode(int window_index) {
	return (kinc_window_mode_t)kinc_internal_windows[window_index].mode;
}
