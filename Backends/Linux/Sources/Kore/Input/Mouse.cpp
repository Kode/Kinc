#include "../pch.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>

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

    log(Info, "setPosition");
    /*
	Display *dpy;
    Window root_window;

    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);
    XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, 100, 100);
    XFlush(dpy); // Flushes the output buffer, therefore updates the cursor's position. Thanks to Achernar.*/
}

void Mouse::getPosition(int& x, int& y){
	Display *dpy;
    Window inwin;      /* root window the pointer is in */
    Window inchildwin; /* child win the pointer is in */
    int rootx, rooty; /* relative to the "root" window; */
    Atom atom_type_prop;
    int actual_format;   /* should be 32 after the call */
    unsigned int mask;   /* status of the buttons */
    unsigned long n_items, bytes_after_ret;
    Window *props;

    /* default DISPLAY */
    dpy = XOpenDisplay(NULL);

    (void)XGetWindowProperty(dpy, DefaultRootWindow(dpy),
               XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True),
               0, 1, False, AnyPropertyType,
               &atom_type_prop, &actual_format,
               &n_items, &bytes_after_ret, (unsigned char**)&props);

    XQueryPointer(dpy, props[0], &inwin,  &inchildwin,
        &rootx, &rooty, &x, &y, &mask);
    int i = 0;
}
