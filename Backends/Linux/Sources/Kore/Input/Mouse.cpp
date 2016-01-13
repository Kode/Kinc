#include "../pch.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#include <GL/glx.h>
#include <GL/gl.h>

#include <X11/X.h>
#include <X11/keysym.h>

using namespace Kore;

void Mouse::_lock(bool truth){
    show(!truth);
    if (truth) {
        int width = System::screenWidth();
        int height = System::screenHeight();

        int x, y;
        getPosition(x, y);

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
        else if (y > height){
            newY -= y - height;
        }

        // Force the mouse to stay inside the window
        setPosition(newX, newY);
    }
}

bool Mouse::canLock(){
	return true;
}

void Mouse::show(bool truth){
	//TODO
}

void Mouse::setPosition(int x, int y){
	Display* dpy = XOpenDisplay(0);
    Window win = (XID)System::windowHandle();

    XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
    XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position.

    XCloseDisplay(dpy);
}

void Mouse::getPosition(int& x, int& y){
    Display* dpy = XOpenDisplay(NULL);
    Window win = (XID)System::windowHandle();

    Window inwin;
    Window inchildwin;
    int rootx, rooty;
    unsigned int mask;

    XQueryPointer(dpy, win, &inwin,  &inchildwin,
        &rootx, &rooty, &x, &y, &mask);

    XCloseDisplay(dpy);
}
