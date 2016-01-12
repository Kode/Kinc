#include "../pch.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#include <stdio.h>
#include <stdlib.h>

#include <GL/glx.h>
#include <GL/gl.h>

#include <X11/X.h>
#include <X11/keysym.h>

using namespace Kore;

void Mouse::_lock(bool truth){
    show(!truth);
    if (truth) {
        int width = System::screenWidth(),
            height = System::screenHeight(),
            x,y, newX, newY;
        getPosition(x, y);

        // Guess the new position of X and Y
        newX = x;
        newY = y;

        // Correct the position of the X coordinate
        // if the mouse is out the window
        if (x < 0) {
            newX -= x;
        } else if (x > width) {
            newX -= x - width;
        }

        // Correct the position of the Y coordinate
        // if the mouse is out the window
        if (y < 0) {
            newY -= y;
        } else if (y > height){
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

	Display *dpy;
    Window win;

    /* Open a connection with X11 server and get the root window */
    dpy = XOpenDisplay(0);
    win = System::getWindow();
    /* Set the pointer position */
    XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
    XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position.
    // Close the opened connection
    XCloseDisplay(dpy);
}

void Mouse::getPosition(int& x, int& y){
	Display *dpy;
    Window win; /* Window used by the application */
    Window inwin;      /* root window the pointer is in */
    Window inchildwin; /* child win the pointer is in */
    int rootx, rooty; /* relative to the "root" window; */
    unsigned int mask;

    /* Open a connection with X11 server and get the root window */
    dpy = XOpenDisplay(NULL);
    win = System::getWindow();

    XQueryPointer(dpy, win, &inwin,  &inchildwin,
        &rootx, &rooty, &x, &y, &mask);

    // Close the opened connection
    XCloseDisplay(dpy);
}
