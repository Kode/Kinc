#include "../pch.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/X.h>
#include <X11/keysym.h>

using namespace Kore;

void Mouse::_lock(int windowId, bool truth) {
	show(!truth);
	if (truth) {
		int width = System::windowWidth(windowId);
		int height = System::windowHeight(windowId);

		int x, y;
		getPosition(windowId, x, y);

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
		setPosition(windowId, newX, newY);
	}
}

bool Mouse::canLock(int windowId) {
	return true;
}

void Mouse::show(bool truth) {
#ifdef KORE_OPENGL
	Display* dpy = XOpenDisplay(0);
	::Window win = (XID)Window::get(0)->_data.handle;
	if (truth) {
		XUndefineCursor(dpy, win);
	}
	else {
		XColor col;
		char data[1] = {0};
		Pixmap blank = XCreateBitmapFromData(dpy, win, data, 1, 1);
		Cursor cursor = XCreatePixmapCursor(dpy, blank, blank, &col, &col, 0, 0);
		XFreePixmap(dpy, blank);
		XDefineCursor(dpy, win, cursor);
	}
#endif
}

void Mouse::setPosition(int windowId, int x, int y) {
#ifdef KORE_OPENGL
	Display* dpy = XOpenDisplay(0);
	::Window win = (XID)Window::get(windowId)->_data.handle;

	XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position.

	XCloseDisplay(dpy);
#endif
}

void Mouse::getPosition(int windowId, int& x, int& y) {
#ifdef KORE_OPENGL
	Display* dpy = XOpenDisplay(NULL);
	::Window win = (XID)Window::get(windowId)->_data.handle;

	::Window inwin;
	::Window inchildwin;
	int rootx, rooty;
	unsigned int mask;

	XQueryPointer(dpy, win, &inwin, &inchildwin, &rootx, &rooty, &x, &y, &mask);

	XCloseDisplay(dpy);
#endif
}
