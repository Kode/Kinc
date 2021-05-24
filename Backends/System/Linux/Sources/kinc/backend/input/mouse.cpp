#include <kinc/backend/Linux.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "../windowdata.h"

#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/Xcursor/Xcursor.h>

#define MAXIMUM_WINDOWS 16
extern Kore::WindowData kinc_internal_windows[MAXIMUM_WINDOWS];

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
    ::Display* dpy = Kore::Linux::display;
    ::Window win = (XID)kinc_internal_windows[0].handle;
    if (_mouseHidden) {
        XUndefineCursor(dpy, win);
        _mouseHidden = false;
    }
}

void kinc_mouse_hide() {
    ::Display* dpy = Kore::Linux::display;
    ::Window win = (XID)kinc_internal_windows[0].handle;
    if (!_mouseHidden) {
        XColor col = XColor{0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0};
        char data[1] = {'\0'};
        Pixmap blank = XCreateBitmapFromData(dpy, win, data, 1, 1);
        Cursor cursor = XCreatePixmapCursor(dpy, blank, blank, &col, &col, 0, 0);
        XDefineCursor(dpy, win, cursor);
        XFreePixmap(dpy, blank);
        _mouseHidden = true;
    }
}

void kinc_mouse_set_cursor(int cursorIndex) {
    ::Display* dpy = Kore::Linux::display;
    ::Window win = (XID)kinc_internal_windows[0].handle;
    if (!_mouseHidden) {
        Cursor cursor;
        switch(cursorIndex) {
            case 0: {
                cursor = XcursorLibraryLoadCursor(dpy, "arrow");
                break;
            }
            case 1: {
                cursor = XcursorLibraryLoadCursor(dpy, "hand1");
                break;
            }
            case 2: {
                cursor = XcursorLibraryLoadCursor(dpy, "xterm");
                break;
            }
            case 3: {
                cursor = XcursorLibraryLoadCursor(dpy, "sb_h_double_arrow");
                break;
            }
            case 4: {
                cursor = XcursorLibraryLoadCursor(dpy, "sb_v_double_arrow");
                break;
            }
            case 5: {
                cursor = XcursorLibraryLoadCursor(dpy, "top_right_corner");
                break;
            }
            case 6: {
                cursor = XcursorLibraryLoadCursor(dpy, "bottom_right_corner");
                break;
            }
            case 7: {
                cursor = XcursorLibraryLoadCursor(dpy, "top_left_corner");
                break;
            }
            case 8: {
                cursor = XcursorLibraryLoadCursor(dpy, "bottom_left_corner");
                break;
            }
            case 9: {
                cursor = XcursorLibraryLoadCursor(dpy, "grab");
                break;
            }
            case 10: {
                cursor = XcursorLibraryLoadCursor(dpy, "grabbing");
                break;
            }
            case 11: {
                cursor = XcursorLibraryLoadCursor(dpy, "not-allowed");
                break;
            }
            case 12: {
                cursor = XcursorLibraryLoadCursor(dpy, "watch");
                break;
            }
            case 13: {
                cursor = XcursorLibraryLoadCursor(dpy, "crosshair");
                break;
            }
            default: {
                cursor = XcursorLibraryLoadCursor(dpy, "arrow");
                break;
            }
        }
        XDefineCursor(dpy, win, cursor);
    }
}

void kinc_mouse_set_position(int window, int x, int y) {
    ::Display* dpy = XOpenDisplay(0);
    ::Window win = (XID)kinc_internal_windows[0].handle;

    XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
    XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position.

    XCloseDisplay(dpy);
}

void kinc_mouse_get_position(int window, int *x, int *y) {
    ::Display* dpy = XOpenDisplay(NULL);
    ::Window win = (XID)kinc_internal_windows[0].handle;

    ::Window inwin;
    ::Window inchildwin;
    int rootx, rooty;
    unsigned int mask;

    XQueryPointer(dpy, win, &inwin, &inchildwin, &rootx, &rooty, x, y, &mask);

    XCloseDisplay(dpy);
}
