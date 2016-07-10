#include "pch.h"
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>

#include "Display.h"

#include <cstring>

#include <stdio.h>
#include <stdlib.h>

#ifdef OPENGL
#include <GL/glx.h>
#include <GL/gl.h>

#include <X11/keysym.h>
//#include <X11/Xlib.h>
#endif

//apt-get install mesa-common-dev
//apt-get install libgl-dev

namespace {
    //static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, None};
    //static int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, None};
#ifdef OPENGL
    struct MwmHints {
        // These correspond to XmRInt resources. (VendorSE.c)
        int	         flags;
        int		 functions;
        int		 decorations;
        int		 input_mode;
        int		 status;
    };

    #define MWM_HINTS_DECORATIONS	(1L << 1)
    Display* dpy;
    GLboolean doubleBuffer = GL_TRUE;
#endif

    void fatalError(const char* message) {
        printf("main: %s\n", message);
        exit(1);
    }
}

#ifdef OPENGL
namespace windowimpl {
    struct KoreWindow : public Kore::KoreWindowBase {
        Window handle;
        GLXContext context;

        KoreWindow( Window handle, GLXContext context, int x, int y, int width, int height ) : KoreWindowBase(x, y, width, height) {
            this->handle = handle;
            this->context = context;
        }
    };

    KoreWindow * windows[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    int windowCounter = -1;

    int idFromWindow( Window window ) {
        for (int windowIndex = 0; windowIndex < sizeof(windows) / sizeof(windows[0]); ++windowIndex) {
            if (windows[windowIndex]->handle == window) {
                return windowIndex;
            }
        }

        return -1;
    }
}
#endif

void Kore::System::setup() {
    Display::enumerate();
}

bool Kore::System::isFullscreen() {
    // TODO (DK)
    return false;
}

#ifndef OPENGL
xcb_connection_t* connection;
xcb_screen_t* screen;
xcb_window_t window;
xcb_intern_atom_reply_t* atom_wm_delete_window;

namespace {
	int windowWidth;
	int windowHeight;
}
#endif

// TODO (DK) the whole glx stuff should go into Graphics/OpenGL?
//  -then there would be a better separation between window + context setup
int createWindow(const char* title, int x, int y, int width, int height, Kore::WindowMode windowMode, int targetDisplay, int depthBufferBits, int stencilBufferBits) {
#ifdef OPENGL
    int wcounter = windowimpl::windowCounter + 1;

	XVisualInfo*         vi;
	Colormap             cmap;
	XSetWindowAttributes swa;
	GLXContext           cx;
	//XEvent               event;
	//GLboolean            needRedraw = GL_FALSE;
	//GLboolean            recalcModelView = GL_TRUE;
	int                  dummy;

	// (1) open a connection to the X server

    if (dpy == nullptr) {
        dpy = XOpenDisplay(NULL);
    }

	if (dpy == NULL) {
        fatalError("could not open display");
    }

	// (2) make sure OpenGL's GLX extension supported
	if (!glXQueryExtension(dpy, &dummy, &dummy)) {
        fatalError("X server has no OpenGL GLX extension");
    }

	// (3) find an appropriate visual
	// find an OpenGL-capable RGB visual with depth buffer
    int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, depthBufferBits, GLX_STENCIL_SIZE, stencilBufferBits, None};
    int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, depthBufferBits, GLX_STENCIL_SIZE, stencilBufferBits, GLX_DOUBLEBUFFER, None};

	vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);

	if (vi == NULL) {
		vi = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);

		if (vi == NULL) {
            fatalError("no RGB visual with valid depth/stencil buffer");
		}

		doubleBuffer = GL_FALSE;
	}
	//if(vi->class != TrueColor)
	//  fatalError("TrueColor visual required for this program");

	// (4) create an OpenGL rendering context
	// TODO (DK)
	//  -context sharing doesn't seem to work in virtual box?
	//      -main screen flickers
	//      -sprite in subscreens is black
	cx = glXCreateContext(dpy, vi, wcounter == 0 ? None : windowimpl::windows[0]->context, /* direct rendering if possible */ GL_TRUE);

	if (cx == NULL) {
        fatalError("could not create rendering context");
	}

	// (5) create an X window with the selected visual

	// create an X colormap since probably not using default visual
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

	Window win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
	XSetStandardProperties(dpy, win, title, "main", None, NULL, 0, NULL);

	switch (windowMode) {
        case Kore::WindowModeFullscreen: // fall through
        case Kore::WindowModeBorderless: {
            Atom awmHints = XInternAtom(dpy, "_MOTIF_WM_HINTS", 0);
            MwmHints hints;
            hints.flags = MWM_HINTS_DECORATIONS;
            hints.decorations = 0;

            XChangeProperty(dpy, win, awmHints, awmHints, 32, PropModeReplace, (unsigned char *)&hints, 5);
        }
    }

	// (6) bind the rendering context to the window
	glXMakeCurrent(dpy, win, cx);

	const Kore::Display::DeviceInfo * deviceInfo = targetDisplay < 0
		? Kore::Display::primaryScreen()
		: Kore::Display::screenById(targetDisplay)
		;

	int dstx = deviceInfo->x;
	int dsty = deviceInfo->y;

	switch (windowMode) {
	    default: // fall through
		case Kore::WindowModeWindow: // fall through
		case Kore::WindowModeBorderless: // fall through
        case Kore::WindowModeFullscreen: {
		    int dw = deviceInfo->width;
		    int dh = deviceInfo->height;
			dstx += x < 0 ? (dw - width) / 2 : x;
			dsty += y < 0 ? (dh - height) / 2 : y;
		} break;
	}

	// (7) request the X window to be displayed on the screen
	XMapWindow(dpy, win);
    XMoveWindow(dpy, win, dstx, dsty);
	//Scheduler::addFrameTask(HandleMessages, 1001);

    windowimpl::windows[wcounter] = new windowimpl::KoreWindow(win, cx, dstx, dsty, width, height);

    Kore::System::makeCurrent(wcounter);

	return windowimpl::windowCounter = wcounter;
#else
	::windowWidth = width;
	::windowHeight = height;

	const xcb_setup_t* setup;
    xcb_screen_iterator_t iter;
    int scr;

    connection = xcb_connect(NULL, &scr);
    if (connection == nullptr) {
        printf("Cannot find a compatible Vulkan installable client driver (ICD).\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    setup = xcb_get_setup(connection);
    iter = xcb_setup_roots_iterator(setup);
	while (scr-- > 0) xcb_screen_next(&iter);

    screen = iter.data;

	window = xcb_generate_id(connection);

	uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t value_list[32];
	value_list[0] = screen->black_pixel;
	value_list[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

    // Magic code that will send notification when window is destroyed
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
    atom_wm_delete_window = xcb_intern_atom_reply(connection, cookie2, 0);

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*reply).atom, 4, 32, 1, &(*atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(connection, window);
	xcb_flush(connection);
    return 1;
#endif
}

namespace Kore { namespace System {
#ifdef OPENGL
    int windowCount() {
        return windowimpl::windowCounter + 1;
    }

    int windowWidth(int id) {
        return windowimpl::windows[id]->width;
    }

    int windowHeight(int id) {
        return windowimpl::windows[id]->height;
    }

    void* windowHandle(int id) {
        return (void*)windowimpl::windows[id]->handle;
    }
#else
	int windowCount() {
        return 1;
    }

    int windowWidth(int id) {
        return ::windowWidth;
    }

    int windowHeight(int id) {
        return ::windowHeight;
    }

    void* windowHandle(int id) {
        return nullptr;
    }
#endif

    int initWindow( WindowOptions options ) {
        char buffer[1024] = {0};
        strcat(buffer, name());
        if (options.title != nullptr) {
            strcat(buffer, options.title);
        }

        int id = createWindow(buffer, options.x, options.y, options.width, options.height, options.mode, options.targetDisplay, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
        Graphics::init(id, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
        return id;
    }
}}

namespace Kore { namespace System {
    int currentDeviceId = -1;

    int currentDevice() {
        return currentDeviceId;
    }

    void setCurrentDevice( int id ) {
        currentDeviceId = id;
    }
}}

bool Kore::System::handleMessages() {
#ifdef OPENGL
	while (XPending(dpy) > 0) {
		XEvent event;
		XNextEvent(dpy, &event);

		switch (event.type) {
		case KeyPress: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);
			#define KEY(xkey, korekey, keychar) case xkey: Kore::Keyboard::the()->_keydown(Kore::korekey, keychar); break;
			switch (keysym) {
			KEY(XK_Right, Key_Right, ' ')
			KEY(XK_Left, Key_Left, ' ')
			KEY(XK_Up, Key_Up, ' ')
			KEY(XK_Down, Key_Down, ' ')
			KEY(XK_space, Key_Space, ' ')
			KEY(XK_BackSpace, Key_Backspace, ' ')
			KEY(XK_Tab, Key_Tab, ' ')
			KEY(XK_Return, Key_Enter, ' ')
			KEY(XK_Shift_L, Key_Shift, ' ')
			KEY(XK_Shift_R, Key_Shift, ' ')
			KEY(XK_Control_L, Key_Control, ' ')
			KEY(XK_Control_R, Key_Control, ' ')
			KEY(XK_Alt_L, Key_Alt, ' ')
			KEY(XK_Alt_R, Key_Alt, ' ')
			KEY(XK_Delete, Key_Delete, ' ')
			KEY(XK_a, Key_A, 'a')
			KEY(XK_b, Key_B, 'b')
			KEY(XK_c, Key_C, 'c')
			KEY(XK_d, Key_D, 'd')
			KEY(XK_e, Key_E, 'e')
			KEY(XK_f, Key_F, 'f')
			KEY(XK_g, Key_G, 'g')
			KEY(XK_h, Key_H, 'h')
			KEY(XK_i, Key_I, 'i')
			KEY(XK_j, Key_J, 'j')
			KEY(XK_k, Key_K, 'k')
			KEY(XK_l, Key_L, 'l')
			KEY(XK_m, Key_M, 'm')
			KEY(XK_n, Key_N, 'n')
			KEY(XK_o, Key_O, 'o')
			KEY(XK_p, Key_P, 'p')
			KEY(XK_q, Key_Q, 'q')
			KEY(XK_r, Key_R, 'r')
			KEY(XK_s, Key_S, 's')
			KEY(XK_t, Key_T, 't')
			KEY(XK_u, Key_U, 'u')
			KEY(XK_v, Key_V, 'v')
			KEY(XK_w, Key_W, 'w')
			KEY(XK_x, Key_X, 'x')
			KEY(XK_y, Key_Y, 'y')
			KEY(XK_z, Key_Z, 'z')
			KEY(XK_1, Key_1, '1')
			KEY(XK_2, Key_1, '2')
			KEY(XK_3, Key_1, '3')
			KEY(XK_4, Key_1, '4')
			KEY(XK_5, Key_1, '5')
			KEY(XK_6, Key_1, '6')
			KEY(XK_7, Key_1, '7')
			KEY(XK_8, Key_1, '8')
			KEY(XK_9, Key_1, '9')
			KEY(XK_0, Key_1, '0')
			case XK_Escape:
                System::stop();
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
			#define KEY(xkey, korekey, keychar) case xkey: Kore::Keyboard::the()->_keyup(Kore::korekey, keychar); break;
			switch (keysym) {
			KEY(XK_Right, Key_Right, ' ')
			KEY(XK_Left, Key_Left, ' ')
			KEY(XK_Up, Key_Up, ' ')
			KEY(XK_Down, Key_Down, ' ')
			KEY(XK_space, Key_Space, ' ')
			KEY(XK_BackSpace, Key_Backspace, ' ')
			KEY(XK_Tab, Key_Tab, ' ')
			KEY(XK_Return, Key_Enter, ' ')
			KEY(XK_Shift_L, Key_Shift, ' ')
			KEY(XK_Shift_R, Key_Shift, ' ')
			KEY(XK_Control_L, Key_Control, ' ')
			KEY(XK_Control_R, Key_Control, ' ')
			KEY(XK_Alt_L, Key_Alt, ' ')
			KEY(XK_Alt_R, Key_Alt, ' ')
			KEY(XK_Delete, Key_Delete, ' ')
			KEY(XK_a, Key_A, 'a')
			KEY(XK_b, Key_B, 'b')
			KEY(XK_c, Key_C, 'c')
			KEY(XK_d, Key_D, 'd')
			KEY(XK_e, Key_E, 'e')
			KEY(XK_f, Key_F, 'f')
			KEY(XK_g, Key_G, 'g')
			KEY(XK_h, Key_H, 'h')
			KEY(XK_i, Key_I, 'i')
			KEY(XK_j, Key_J, 'j')
			KEY(XK_k, Key_K, 'k')
			KEY(XK_l, Key_L, 'l')
			KEY(XK_m, Key_M, 'm')
			KEY(XK_n, Key_N, 'n')
			KEY(XK_o, Key_O, 'o')
			KEY(XK_p, Key_P, 'p')
			KEY(XK_q, Key_Q, 'q')
			KEY(XK_r, Key_R, 'r')
			KEY(XK_s, Key_S, 's')
			KEY(XK_t, Key_T, 't')
			KEY(XK_u, Key_U, 'u')
			KEY(XK_v, Key_V, 'v')
			KEY(XK_w, Key_W, 'w')
			KEY(XK_x, Key_X, 'x')
			KEY(XK_y, Key_Y, 'y')
			KEY(XK_z, Key_Z, 'z')
			KEY(XK_1, Key_1, '1')
			KEY(XK_2, Key_1, '2')
			KEY(XK_3, Key_1, '3')
			KEY(XK_4, Key_1, '4')
			KEY(XK_5, Key_1, '5')
			KEY(XK_6, Key_1, '6')
			KEY(XK_7, Key_1, '7')
			KEY(XK_8, Key_1, '8')
			KEY(XK_9, Key_1, '9')
			KEY(XK_0, Key_1, '0')
			}
			break;
			#undef KEY
		}
		case ButtonPress: {
			XButtonEvent* button = (XButtonEvent*)&event;
			int windowId = windowimpl::idFromWindow(button->window);

			switch (button->button) {
			case Button1:
                Kore::Mouse::the()->_press(windowId, 0, button->x, button->y);
                break;
            case Button3:
                Kore::Mouse::the()->_press(windowId, 1, button->x, button->y);
                break;
			}
			break;
		}
		case ButtonRelease: {
			XButtonEvent* button = (XButtonEvent*)&event;
			int windowId = windowimpl::idFromWindow(button->window);

			switch (button->button) {
			case Button1:
                Kore::Mouse::the()->_release(windowId, 0, button->x, button->y);
                break;
            case Button3:
                Kore::Mouse::the()->_release(windowId, 1, button->x, button->y);
                break;
			}
			break;
		}
		case MotionNotify: {
			XMotionEvent* motion = (XMotionEvent*)&event;
			int windowId = windowimpl::idFromWindow(motion->window);
			Kore::Mouse::the()->_move(windowId, motion->x, motion->y);
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
#else
	xcb_generic_event_t* event = xcb_poll_for_event(connection);
	while (event != nullptr) {
		switch (event->response_type & 0x7f) {
		case XCB_EXPOSE:
			break;
		case XCB_CLIENT_MESSAGE:
			if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*atom_wm_delete_window).atom) {
				exit(0);
			}
			break;
		case XCB_KEY_PRESS: {
			const xcb_key_press_event_t* key = (const xcb_key_press_event_t*)event;
			switch (key->detail) {
			case 111: Kore::Keyboard::the()->_keydown(Kore::Key_Up, ' '); break;
			case 116: Kore::Keyboard::the()->_keydown(Kore::Key_Down, ' '); break;
			case 113: Kore::Keyboard::the()->_keydown(Kore::Key_Left, ' '); break;
			case 114: Kore::Keyboard::the()->_keydown(Kore::Key_Right, ' '); break;
			}
			break;
		}
		case XCB_KEY_RELEASE: {
			const xcb_key_release_event_t* key = (const xcb_key_release_event_t*)event;
			if (key->detail == 0x9) exit(0);
			switch (key->detail) {
			case 111: Kore::Keyboard::the()->_keyup(Kore::Key_Up, ' '); break;
			case 116: Kore::Keyboard::the()->_keyup(Kore::Key_Down, ' '); break;
			case 113: Kore::Keyboard::the()->_keyup(Kore::Key_Left, ' '); break;
			case 114: Kore::Keyboard::the()->_keyup(Kore::Key_Right, ' '); break;
			}
			break;
		}
		case XCB_DESTROY_NOTIFY:
			exit(0);
			break;
		case XCB_CONFIGURE_NOTIFY: {
			const xcb_configure_notify_event_t* cfg = (const xcb_configure_notify_event_t*)event;
			//if ((demo->width != cfg->width) || (demo->height != cfg->height)) {
			//	demo->width = cfg->width;
			//	demo->height = cfg->height;
			//	demo_resize(demo);
			//}
			break;
		}
		default:
			break;
		}
		free(event);
		event = xcb_poll_for_event(connection);
	}
#endif
	return true;
}

const char* Kore::System::systemId() {
    return "Linux";
}

void Kore::System::makeCurrent( int contextId ) {
	if (currentDeviceId == contextId) {
		return;
	}
    currentDeviceId = contextId;
#ifdef OPENGL
	glXMakeCurrent(dpy, windowimpl::windows[contextId]->handle, windowimpl::windows[contextId]->context);
#endif
}

void Kore::Graphics::clearCurrent() {
}

void Kore::System::clearCurrent() {
    currentDeviceId = -1;
    Graphics::clearCurrent();
}

void Kore::System::swapBuffers( int contextId ) {
#ifdef OPENGL
    glXSwapBuffers(dpy, windowimpl::windows[contextId]->handle);
#endif
}

void Kore::System::destroyWindow( int id ) {
    // TODO (DK) implement me
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {

}

void Kore::System::setTitle(const char* title) {

}

void Kore::System::setKeepScreenOn( bool on ) {

}

void Kore::System::showWindow() {

}

void Kore::System::showKeyboard() {

}

void Kore::System::hideKeyboard() {

}

void Kore::System::loadURL(const char* url) {

}

int Kore::System::desktopWidth() {
#ifdef OPENGL
    return XWidthOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
    return 1920;
#endif
}

int Kore::System::desktopHeight() {
#ifdef OPENGL
    return XHeightOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
	return 1080;
#endif
}

namespace {
    char save[2000];
    bool saveInitialized = false;
}

const char* Kore::System::savePath() {
    if (!saveInitialized) {
        strcpy(save, "Ķ~/.");
        strcat(save, name());
        strcat(save, "/");
        saveInitialized = true;
    }
	return save;
}

namespace {
	const char* videoFormats[] = { "ogv", nullptr };
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
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
