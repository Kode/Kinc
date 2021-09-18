#include <kinc/backend/Linux.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "windowdata.h"

#include <X11/X.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/keysym.h>

#define MAXIMUM_WINDOWS 16
extern struct KincWindowData kinc_internal_windows[MAXIMUM_WINDOWS];

void kinc_internal_mouse_lock(int window) {
	kinc_mouse_hide();
	int width = kinc_window_width(window);
	int height = kinc_window_height(window);

	int x, y;
	kinc_mouse_get_position(window, &x, &y);

	// Guess the new position of X and Y
	int newX = x;
	int newY = y;

	// Correct the position of the X coordinate
	// if the mouse is out the window
	if (x < 0) {
		newX -= x;
	}
	else if (x > width) {
		newX -= x - width;
	}

	// Correct the position of the Y coordinate
	// if the mouse is out the window
	if (y < 0) {
		newY -= y;
	}
	else if (y > height) {
		newY -= y - height;
	}

	// Force the mouse to stay inside the window
	kinc_mouse_set_position(window, newX, newY);
}

void kinc_internal_mouse_unlock(void) {
	kinc_mouse_show();
}

bool kinc_mouse_can_lock(void) {
	return true;
}

bool _mouseHidden = false;

void kinc_mouse_show() {
	Window win = (XID)kinc_internal_windows[0].handle;
	if (_mouseHidden) {
		XUndefineCursor(kinc_linux_display, win);
		_mouseHidden = false;
	}
}

void kinc_mouse_hide() {
	Window win = (XID)kinc_internal_windows[0].handle;
	if (!_mouseHidden) {
		XColor col;
		col.pixel = 0;
		col.red = 0;
		col.green = 0;
		col.blue = 0;
		col.flags = DoRed | DoGreen | DoBlue;
		col.pad = 0;
		char data[1] = {'\0'};
		Pixmap blank = XCreateBitmapFromData(kinc_linux_display, win, data, 1, 1);
		Cursor cursor = XCreatePixmapCursor(kinc_linux_display, blank, blank, &col, &col, 0, 0);
		XDefineCursor(kinc_linux_display, win, cursor);
		XFreePixmap(kinc_linux_display, blank);
		_mouseHidden = true;
	}
}

void kinc_mouse_set_cursor(int cursorIndex) {
	Window win = (XID)kinc_internal_windows[0].handle;
	if (!_mouseHidden) {
		Cursor cursor;
		switch (cursorIndex) {
		case 0: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "arrow");
			break;
		}
		case 1: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "hand1");
			break;
		}
		case 2: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "xterm");
			break;
		}
		case 3: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "sb_h_double_arrow");
			break;
		}
		case 4: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "sb_v_double_arrow");
			break;
		}
		case 5: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "top_right_corner");
			break;
		}
		case 6: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "bottom_right_corner");
			break;
		}
		case 7: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "top_left_corner");
			break;
		}
		case 8: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "bottom_left_corner");
			break;
		}
		case 9: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "grab");
			break;
		}
		case 10: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "grabbing");
			break;
		}
		case 11: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "not-allowed");
			break;
		}
		case 12: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "watch");
			break;
		}
		case 13: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "crosshair");
			break;
		}
		default: {
			cursor = XcursorLibraryLoadCursor(kinc_linux_display, "arrow");
			break;
		}
		}
		XDefineCursor(kinc_linux_display, win, cursor);
	}
}

void kinc_mouse_set_position(int window, int x, int y) {
	Display *dpy = XOpenDisplay(0);
	Window win = (XID)kinc_internal_windows[0].handle;

	XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position.

	XCloseDisplay(dpy);
}

void kinc_mouse_get_position(int window, int *x, int *y) {
	Display *dpy = XOpenDisplay(NULL);
	Window win = (XID)kinc_internal_windows[0].handle;

	Window inwin;
	Window inchildwin;
	int rootx, rooty;
	unsigned int mask;

	XQueryPointer(dpy, win, &inwin, &inchildwin, &rootx, &rooty, x, y, &mask);

	XCloseDisplay(dpy);
}
