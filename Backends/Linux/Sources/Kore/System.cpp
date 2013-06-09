#include "pch.h"
#include <Kore/System.h>
#include <cstring>
#include <Kore/Application.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/KeyEvent.h>
#include <Kore/Input/Mouse.h>

#include <stdio.h>
#include <stdlib.h>

#include <GL/glx.h>
#include <GL/gl.h>

#include <X11/X.h>
#include <X11/keysym.h>

//apt-get install mesa-common-dev
//apt-get install libgl-dev

namespace {
    static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, None};
    static int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};

    Display   *dpy;
    Window     win;
    GLboolean  doubleBuffer = GL_TRUE;

    void fatalError(const char* message) {
        printf("main: %s\n", message);
        exit(1);
    }
}

using namespace Kore;

void* System::createWindow() {
	XVisualInfo*         vi;
	Colormap             cmap;
	XSetWindowAttributes swa;
	GLXContext           cx;
	//XEvent               event;
	//GLboolean            needRedraw = GL_FALSE;
	//GLboolean            recalcModelView = GL_TRUE;
	int                  dummy;

	// (1) open a connection to the X server

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) fatalError("could not open display");

	// (2) make sure OpenGL's GLX extension supported

	if (!glXQueryExtension(dpy, &dummy, &dummy)) fatalError("X server has no OpenGL GLX extension");

	// (3) find an appropriate visual

	// find an OpenGL-capable RGB visual with depth buffer
	vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
	if (vi == NULL) {
		vi = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);
		if (vi == NULL) fatalError("no RGB visual with depth buffer");
		doubleBuffer = GL_FALSE;
	}
	//if(vi->class != TrueColor)
	//  fatalError("TrueColor visual required for this program");

	// (4) create an OpenGL rendering context

	// create an OpenGL rendering context
	cx = glXCreateContext(dpy, vi, /* no shared dlists */ None, /* direct rendering if possible */ GL_TRUE);
	if (cx == NULL) fatalError("could not create rendering context");

	// (5) create an X window with the selected visual

	// create an X colormap since probably not using default visual
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
	win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, Application::the()->width(), Application::the()->height(), 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
	XSetStandardProperties(dpy, win, "main", "main", None, NULL, 0, NULL);

	// (6) bind the rendering context to the window

	glXMakeCurrent(dpy, win, cx);

	// (7) request the X window to be displayed on the screen

	XMapWindow(dpy, win);

	//Scheduler::addFrameTask(HandleMessages, 1001);

	return nullptr;
}

bool System::handleMessages() {
	while (XPending(dpy) > 0) {
		XEvent event;
		XNextEvent(dpy, &event);
		switch (event.type) {
		case KeyPress: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);
			switch (keysym) {
			case XK_Right:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Right));
				break;
			case XK_Left:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Left));
				break;
			case XK_Up:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Up));
				break;
			case XK_Down:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Down));
				break;
			case XK_Escape:
				Application::the()->stop();
				break;
			}
			break;
		}
		case KeyRelease: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);
			switch (keysym) {
			case XK_Right:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Right));
				break;
			case XK_Left:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Left));
				break;
			case XK_Up:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Up));
				break;
			case XK_Down:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Down));
				break;
			}
			break;
		}
		case ButtonPress: {
			XButtonEvent* button = (XButtonEvent*)&event;
			Kore::Mouse::the()->_pressLeft(Kore::MouseEvent(button->x, button->y));
			break;
		}
		case ButtonRelease: {
			XButtonEvent* button = (XButtonEvent*)&event;
			Kore::Mouse::the()->_releaseLeft(Kore::MouseEvent(button->x, button->y));
			break;
		}
		case MotionNotify: {
			XMotionEvent* motion = (XMotionEvent*)&event;
			Kore::Mouse::the()->_move(Kore::MouseEvent(motion->x, motion->y));
			break;
		}
		case ConfigureNotify:
			glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
			// fall through...
		case Expose:
			//needRedraw = GL_TRUE;
			break;
		}
	}
	return true;
}

void Kore::System::swapBuffers() {
    glXSwapBuffers(dpy, win);
}

void Kore::System::destroyWindow() {

}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {

}

void Kore::System::setTitle(const char* title) {

}

void Kore::System::showWindow() {

}

#include <sys/time.h>
#include <time.h>

double Kore::System::frequency() {
	return 1000000.0;
}

Kore::System::ticks Kore::System::timestamp() {
	timeval now;
	gettimeofday(&now, NULL);
	return static_cast<ticks>(now.tv_sec) * 1000000 + static_cast<ticks>(now.tv_usec);
}

extern int kore(int argc, char** argv);

int main(int argc, char** argv) {
    kore(argc, argv);
}
