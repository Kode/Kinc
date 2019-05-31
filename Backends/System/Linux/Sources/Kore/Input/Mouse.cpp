#include "../pch.h"

#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "../WindowData.h"

#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/X.h>
#include <X11/keysym.h>

#define MAXIMUM_WINDOWS 16
extern Kore::WindowData kinc_internal_windows[MAXIMUM_WINDOWS];

void Kinc_Mouse_Lock(int window) {
    Kinc_Mouse_Hide();
    int width = Kinc_WindowWidth(window);
    int height = Kinc_WindowHeight(window);

    int x, y;
    Kinc_Mouse_GetPosition(window, &x, &y);

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
    Kinc_Mouse_SetPosition(window, newX, newY);
}

void Kinc_Mouse_Unlock(int window) {
    Kinc_Mouse_Show();
}

bool Kinc_Mouse_CanLock(int window) {
	return true;
}

bool _mouseHidden = false;

void Kinc_Mouse_Show() {
#ifdef KORE_OPENGL
	::Display* dpy = glXGetCurrentDisplay();
	::Window win = (XID)kinc_internal_windows[0].handle;
	if (_mouseHidden)
    {
		//log(LogLevel::Info, "show mouse\n");
        XUndefineCursor(dpy, win);
		_mouseHidden = false;
    }
#endif
}

void Kinc_Mouse_Hide() {
#ifdef KORE_OPENGL
    ::Display* dpy = glXGetCurrentDisplay();
    ::Window win = (XID)kinc_internal_windows[0].handle;
    if (!_mouseHidden)
    {
        //log(LogLevel::Info, "hide mouse\n");
        XColor col = XColor{0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0};
        char data[1] = {'\0'};
        Pixmap blank = XCreateBitmapFromData(dpy, win, data, 1, 1);
        Cursor cursor = XCreatePixmapCursor(dpy, blank, blank, &col, &col, 0, 0);
        XDefineCursor(dpy, win, cursor);
        XFreePixmap(dpy, blank);
        _mouseHidden = true;
    }
#endif
}

void Kinc_Mouse_SetPosition(int window, int x, int y) {
#ifdef KORE_OPENGL
	::Display* dpy = XOpenDisplay(0);
	::Window win = (XID)kinc_internal_windows[0].handle;

	XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position.

	XCloseDisplay(dpy);
#endif
}

void Kinc_Mouse_GetPosition(int window, int *x, int *y) {
#ifdef KORE_OPENGL
	::Display* dpy = XOpenDisplay(NULL);
	::Window win = (XID)kinc_internal_windows[0].handle;

	::Window inwin;
	::Window inchildwin;
	int rootx, rooty;
	unsigned int mask;

	XQueryPointer(dpy, win, &inwin, &inchildwin, &rootx, &rooty, x, y, &mask);

	XCloseDisplay(dpy);
#endif
}
