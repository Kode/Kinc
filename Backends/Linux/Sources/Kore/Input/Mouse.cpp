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
	//TODO
}


bool Mouse::canLock(){
	return true; //FOR NOW
}


void Mouse::show(bool truth){
	//TODO
    log(Info, "show called");
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
}

void Mouse::getPosition(int& x, int& y){
	Display *dpy;
    Window win; /* Window used by the application */
    Window inwin;      /* root window the pointer is in */
    Window inchildwin; /* child win the pointer is in */
    int rootx, rooty; /* relative to the "root" window; */
    Atom atom_type_prop;
    unsigned int mask;

    /* Open a connection with X11 server and get the root window */
    dpy = XOpenDisplay(NULL);
    win = System::getWindow();

    XQueryPointer(dpy, win, &inwin,  &inchildwin,
        &rootx, &rooty, &x, &y, &mask);

    // Close the opened connection
    XCloseDisplay(dpy);
}
