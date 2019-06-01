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

int Kinc_CountWindows(void) {
	return 1;
}

int Kinc_WindowX(int window_index) {
	return 0;
}

int Kinc_WindowY(int window_index) {
	return 0;
}

int Kinc_WindowWidth(int window_index) {
    return kinc_internal_windows[window_index].width;
}

int Kinc_WindowHeight(int window_index) {
    return kinc_internal_windows[window_index].height;
}

void Kinc_WindowResize(int window_index, int width, int height) {

}

void Kinc_WindowMove(int window_index, int x, int y) {

}

void Kinc_WindowChangeFramebuffer(int window_index, Kinc_FramebufferOptions *frame) {
	//**kinc_g4_changeFramebuffer(0, frame);
}

void Kinc_WindowChangeFeatures(int window_index, int features) {

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

void Kinc_WindowChangeMode(int window_index, Kinc_WindowMode mode) {
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

int Kinc_WindowDisplay(int window_index) {
	return NULL;
}

void Kinc_WindowDestroy(int window_index) {

}

void Kinc_WindowShow(int window_index) {

}

void Kinc_WindowHide(int window_index) {

}

void Kinc_WindowSetTitle(int window_index, const char *title) {

}

int Kinc_WindowCreate(Kinc_WindowOptions *win, Kinc_FramebufferOptions *frame) {
	return 0;
}

void Kinc_WindowSetResizeCallback(int window_index, void (*callback)(int x, int y, void *data), void *data) {

}

void Kinc_WindowSetPpiChangedCallback(int window_index, void (*callback)(int ppi, void *data), void *data) {

}

Kinc_WindowMode Kinc_WindowGetMode(int window_index) {
	return (Kinc_WindowMode)kinc_internal_windows[window_index].mode;
}
