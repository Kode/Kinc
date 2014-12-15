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

    Display* dpy;
    Window win;
    GLboolean doubleBuffer = GL_TRUE;

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
    XMoveWindow(dpy, win, DisplayWidth(dpy, vi->screen) / 2 - Application::the()->width() / 2, DisplayHeight(dpy, vi->screen) / 2 - Application::the()->height() / 2);
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
			#define KEY(xkey, korekey) case xkey: Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::korekey)); break;
			switch (keysym) {
			KEY(XK_Right, Key_Right)
			KEY(XK_Left, Key_Left)
			KEY(XK_Up, Key_Up)
			KEY(XK_Down, Key_Down)
			KEY(XK_space, Key_Space)
			KEY(XK_a, Key_A)
			KEY(XK_b, Key_B)
			KEY(XK_c, Key_C)
			KEY(XK_d, Key_D)
			KEY(XK_e, Key_E)
			KEY(XK_f, Key_F)
			KEY(XK_g, Key_G)
			KEY(XK_h, Key_H)
			KEY(XK_i, Key_I)
			KEY(XK_j, Key_J)
			KEY(XK_k, Key_K)
			KEY(XK_l, Key_L)
			KEY(XK_m, Key_M)
			KEY(XK_n, Key_N)
			KEY(XK_o, Key_O)
			KEY(XK_p, Key_P)
			KEY(XK_q, Key_Q)
			KEY(XK_r, Key_R)
			KEY(XK_s, Key_S)
			KEY(XK_t, Key_T)
			KEY(XK_u, Key_U)
			KEY(XK_v, Key_V)
			KEY(XK_w, Key_W)
			KEY(XK_x, Key_X)
			KEY(XK_y, Key_Y)
			KEY(XK_z, Key_Z)
			case XK_Escape:
				Application::the()->stop();
				break;
			}
			break;
			#undef KEY
		}
		case KeyRelease: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);
			#define KEY(xkey, korekey) case xkey: Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::korekey)); break;
			switch (keysym) {
			KEY(XK_Right, Key_Right)
			KEY(XK_Left, Key_Left)
			KEY(XK_Up, Key_Up)
			KEY(XK_Down, Key_Down)
			KEY(XK_space, Key_Space)
			KEY(XK_a, Key_A)
			KEY(XK_b, Key_B)
			KEY(XK_c, Key_C)
			KEY(XK_d, Key_D)
			KEY(XK_e, Key_E)
			KEY(XK_f, Key_F)
			KEY(XK_g, Key_G)
			KEY(XK_h, Key_H)
			KEY(XK_i, Key_I)
			KEY(XK_j, Key_J)
			KEY(XK_k, Key_K)
			KEY(XK_l, Key_L)
			KEY(XK_m, Key_M)
			KEY(XK_n, Key_N)
			KEY(XK_o, Key_O)
			KEY(XK_p, Key_P)
			KEY(XK_q, Key_Q)
			KEY(XK_r, Key_R)
			KEY(XK_s, Key_S)
			KEY(XK_t, Key_T)
			KEY(XK_u, Key_U)
			KEY(XK_v, Key_V)
			KEY(XK_w, Key_W)
			KEY(XK_x, Key_X)
			KEY(XK_y, Key_Y)
			KEY(XK_z, Key_Z)
			}
			break;
			#undef KEY
		}
		case ButtonPress: {
			XButtonEvent* button = (XButtonEvent*)&event;
			switch (button->button) {
			case Button1:
                Kore::Mouse::the()->_press(0, button->x, button->y);
                break;
            case Button3:
                Kore::Mouse::the()->_press(1, button->x, button->y);
                break;
			}
			break;
		}
		case ButtonRelease: {
			XButtonEvent* button = (XButtonEvent*)&event;
			switch (button->button) {
			case Button1:
                Kore::Mouse::the()->_release(0, button->x, button->y);
                break;
            case Button3:
                Kore::Mouse::the()->_release(1, button->x, button->y);
                break;
			}
			break;
		}
		case MotionNotify: {
			XMotionEvent* motion = (XMotionEvent*)&event;
			Kore::Mouse::the()->_move(motion->x, motion->y);
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

void Kore::System::showKeyboard() {

}

void Kore::System::hideKeyboard() {

}

void Kore::System::loadURL(const char* url) {

}

int Kore::System::screenWidth() {
    return Application::the()->width();
}

int Kore::System::screenHeight() {
    return Application::the()->height();
}

namespace {
    char save[2000];
    bool saveInitialized = false;
}

const char* Kore::System::savePath() {
    if (!saveInitialized) {
        strcpy(save, "Ķ~/.");
        strcat(save, Application::the()->name());
        strcat(save, "/");
        saveInitialized = true;
    }
	return save;
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
