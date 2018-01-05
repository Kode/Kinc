#include "pch.h"
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#include "Input/Gamepad.h"

#include "Display.h"

#include <cstring>

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>

#ifdef KORE_OPENGL
#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <climits>
#include <assert.h>
//#include <X11/Xlib.h>
#endif

// apt-get install mesa-common-dev
// apt-get install libgl-dev

namespace {
// static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, None};
// static int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, None};
#ifdef KORE_OPENGL
	struct MwmHints {
		// These correspond to XmRInt resources. (VendorSE.c)
		int flags;
		int functions;
		int decorations;
		int input_mode;
		int status;
	};

#define MWM_HINTS_DECORATIONS (1L << 1)
	Display* dpy;
	GLboolean doubleBuffer = GL_TRUE;
#endif
	Window win;
	Atom XdndDrop;
	Atom XdndFinished;
	Atom XdndActionCopy;
	Atom XdndSelection;
	Atom XdndPrimary;
	Atom clipboard;
	Atom utf8;
	Atom xseldata;
	Window XdndSourceWindow = None;

	void fatalError(const char* message) {
		printf("main: %s\n", message);
		exit(1);
	}
}

#ifdef KORE_OPENGL
namespace windowimpl {
	struct KoreWindow : public Kore::KoreWindowBase {
		Window handle;
		GLXContext context;

		KoreWindow(Window handle, GLXContext context, int x, int y, int width, int height) : KoreWindowBase(x, y, width, height) {
			this->handle = handle;
			this->context = context;
		}
	};

	KoreWindow* windows[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	int windowCounter = -1;

	int idFromWindow(Window window) {
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

#ifndef KORE_OPENGL
xcb_connection_t* connection;
xcb_screen_t* screen;
xcb_window_t window;
xcb_intern_atom_reply_t* atom_wm_delete_window;

namespace {
	int windowWidth;
	int windowHeight;
}
#endif

int createWindow(const char* title, int x, int y, int width, int height, Kore::WindowMode windowMode, int targetDisplay, int depthBufferBits,
                 int stencilBufferBits, int antialiasingSamples) {

	int nameLength = strlen(Kore::System::name());
	char strName[nameLength+1];
	strcpy(strName, Kore::System::name());
	char strClass[] = "_KoreApplication";
	char strNameClass[nameLength+17];
	strcpy(strNameClass, strName);
	strncat(strNameClass, strClass, 16);
	strNameClass[nameLength] = '\0';

#ifdef KORE_OPENGL
	int wcounter = windowimpl::windowCounter + 1;

	XVisualInfo* vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	GLXContext cx;
	// XEvent               event;
	// GLboolean            needRedraw = GL_FALSE;
	// GLboolean            recalcModelView = GL_TRUE;
	int dummy;

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

	int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, depthBufferBits, GLX_STENCIL_SIZE, stencilBufferBits, None};
	int dblBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, depthBufferBits, GLX_STENCIL_SIZE, stencilBufferBits, GLX_DOUBLEBUFFER, None};

	vi = NULL;
	if (antialiasingSamples > 1) {
        int attribs[] = {GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        GLX_SAMPLE_BUFFERS  , 1,
        GLX_SAMPLES         , antialiasingSamples,
        None};

        GLXFBConfig fbconfig = 0;
        int fbcount;
        GLXFBConfig* fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), attribs, &fbcount);
        if (fbc) {
            if (fbcount >= 1) {
                fbconfig = fbc[0];
            }
            XFree(fbc);
        }

        if (fbconfig) {
            vi = glXGetVisualFromFBConfig(dpy, fbconfig);
        }
	}

	if (vi == NULL) {
        vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
    }

	if (vi == NULL) {
		vi = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);

		if (vi == NULL) {
			fatalError("no RGB visual with valid depth/stencil buffer");
		}

		doubleBuffer = GL_FALSE;
	}
	// if(vi->class != TrueColor)
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

	win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
	                           CWBorderPixel | CWColormap | CWEventMask, &swa);
	XSetStandardProperties(dpy, win, title, "main", None, NULL, 0, NULL);

	char* strNameClass_ptr = strNameClass;
	Atom wmClassAtom = XInternAtom( dpy, "WM_CLASS", 0);
	XChangeProperty(dpy, win, wmClassAtom, XA_STRING, 8, PropModeReplace, (unsigned char*)strNameClass_ptr, nameLength+17);

	switch (windowMode) {
	case Kore::WindowModeFullscreen: // fall through
	case Kore::WindowModeBorderless: {
		Atom awmHints = XInternAtom(dpy, "_MOTIF_WM_HINTS", 0);
		MwmHints hints;
		hints.flags = MWM_HINTS_DECORATIONS;
		hints.decorations = 0;

		XChangeProperty(dpy, win, awmHints, awmHints, 32, PropModeReplace, (unsigned char*)&hints, 5);
	}
	}

	// (6) bind the rendering context to the window
	glXMakeCurrent(dpy, win, cx);

	const Kore::Display::DeviceInfo* deviceInfo = targetDisplay < 0 ? Kore::Display::primaryScreen() : Kore::Display::screenById(targetDisplay);

	int dstx = deviceInfo->x;
	int dsty = deviceInfo->y;

	switch (windowMode) {
	default:                         // fall through
	case Kore::WindowModeWindow:     // fall through
	case Kore::WindowModeBorderless: // fall through
	case Kore::WindowModeFullscreen: {
		int dw = deviceInfo->width;
		int dh = deviceInfo->height;
		dstx += x < 0 ? (dw - width) / 2 : x;
		dsty += y < 0 ? (dh - height) / 2 : y;
	} break;
	}

	// (7) request the X window to be displayed on the screen
	if (Kore::System::hasShowWindowFlag()) {
		XMapWindow(dpy, win);
		XMoveWindow(dpy, win, dstx, dsty);
	}
	// Scheduler::addFrameTask(HandleMessages, 1001);

	// Drag and drop
	Atom XdndAware = XInternAtom(dpy, "XdndAware", 0);
	Atom XdndVersion = 5;
	XChangeProperty(dpy, win, XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&XdndVersion, 1);
	XdndDrop = XInternAtom(dpy, "XdndDrop", 0);
	XdndFinished = XInternAtom(dpy, "XdndFinished", 0);
	XdndActionCopy = XInternAtom(dpy, "XdndActionCopy", 0);
	XdndSelection = XInternAtom(dpy, "XdndSelection", 0);
	XdndPrimary = XInternAtom(dpy, "PRIMARY", 0);
	clipboard = XInternAtom(dpy, "CLIPBOARD", 0);
	utf8 = XInternAtom(dpy, "UTF8_STRING", 0);
	xseldata = XInternAtom(dpy, "XSEL_DATA", False),

	//**XSetSelectionOwner(dpy, clipboard, win, CurrentTime);

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

	xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
	                  value_mask, value_list);

	// Needs to be tested
	xcb_intern_atom_cookie_t atom_wm_class_cookie = xcb_intern_atom(connection, 1, 8, "WM_CLASS");
	xcb_intern_atom_reply_t* atom_wm_class_reply = xcb_intern_atom_reply(connection, atom_wm_class_cookie, 0);
	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*atom_wm_class_reply).atom, 31, 8, nameLength+17, strNameClass);
	free(atom_wm_class_reply);

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

namespace Kore {
	namespace System {
#ifdef KORE_OPENGL
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

		int initWindow(WindowOptions options) {
			char buffer[1024] = {0};
			strcpy(buffer, name());
			if (options.title != nullptr) {
				strcpy(buffer, options.title);
			}

			int id = createWindow(buffer, options.x, options.y, options.width, options.height, options.mode, options.targetDisplay,
			                      options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits, options.rendererOptions.antialiasing);
			Graphics4::init(id, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
			return id;
		}
	}
}

namespace Kore {
	namespace System {
		int currentDeviceId = -1;

		int currentDevice() {
			return currentDeviceId;
		}

		void setCurrentDevice(int id) {
			currentDeviceId = id;
		}
	}
}

bool Kore::System::handleMessages() {
	static bool controlDown = false;
#ifdef KORE_OPENGL
	while (XPending(dpy) > 0) {
		XEvent event;
		XNextEvent(dpy, &event);

		switch (event.type) {
		case KeyPress: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);
#define KEY(xkey, korekey)                                    \
	case xkey:                                                \
		Kore::Keyboard::the()->_keydown(Kore::korekey);       \
		break;
			if (keysym == XK_Control_L || keysym == XK_Control_R) {
				controlDown = true;
			}
			else if (controlDown && (keysym == XK_v || keysym == XK_V)) {
				XConvertSelection(dpy, clipboard, utf8, xseldata, win, CurrentTime);
			}
			else if (controlDown && (keysym == XK_c || keysym == XK_C)) {
				XSetSelectionOwner(dpy, clipboard, win, CurrentTime);
			}

			switch (keysym) {
				KEY(XK_Right, KeyRight)
				KEY(XK_Left, KeyLeft)
				KEY(XK_Up, KeyUp)
				KEY(XK_Down, KeyDown)
				KEY(XK_space, KeySpace)
				KEY(XK_BackSpace, KeyBackspace)
				KEY(XK_Tab, KeyTab)
				KEY(XK_Return, KeyReturn)
				KEY(XK_Shift_L, KeyShift)
				KEY(XK_Shift_R, KeyShift)
				KEY(XK_Control_L, KeyControl)
				KEY(XK_Control_R, KeyControl)
				KEY(XK_Alt_L, KeyAlt)
				KEY(XK_Alt_R, KeyAlt)
				KEY(XK_Delete, KeyDelete)
				KEY(XK_a, KeyA)
				KEY(XK_b, KeyB)
				KEY(XK_c, KeyC)
				KEY(XK_d, KeyD)
				KEY(XK_e, KeyE)
				KEY(XK_f, KeyF)
				KEY(XK_g, KeyG)
				KEY(XK_h, KeyH)
				KEY(XK_i, KeyI)
				KEY(XK_j, KeyJ)
				KEY(XK_k, KeyK)
				KEY(XK_l, KeyL)
				KEY(XK_m, KeyM)
				KEY(XK_n, KeyN)
				KEY(XK_o, KeyO)
				KEY(XK_p, KeyP)
				KEY(XK_q, KeyQ)
				KEY(XK_r, KeyR)
				KEY(XK_s, KeyS)
				KEY(XK_t, KeyT)
				KEY(XK_u, KeyU)
				KEY(XK_v, KeyV)
				KEY(XK_w, KeyW)
				KEY(XK_x, KeyX)
				KEY(XK_y, KeyY)
				KEY(XK_z, KeyZ)
				KEY(XK_1, Key1)
				KEY(XK_2, Key2)
				KEY(XK_3, Key3)
				KEY(XK_4, Key4)
				KEY(XK_5, Key5)
				KEY(XK_6, Key6)
				KEY(XK_7, Key7)
				KEY(XK_8, Key8)
				KEY(XK_9, Key9)
				KEY(XK_0, Key0)
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
#define KEY(xkey, korekey)                                \
	case xkey:                                            \
		Kore::Keyboard::the()->_keyup(Kore::korekey);     \
		break;
			if (keysym == XK_Control_L || keysym == XK_Control_R) {
				controlDown = false;
			}

			switch (keysym) {
				KEY(XK_Right, KeyRight)
				KEY(XK_Left, KeyLeft)
				KEY(XK_Up, KeyUp)
				KEY(XK_Down, KeyDown)
				KEY(XK_space, KeySpace)
				KEY(XK_BackSpace, KeyBackspace)
				KEY(XK_Tab, KeyTab)
				KEY(XK_Return, KeyReturn)
				KEY(XK_Shift_L, KeyShift)
				KEY(XK_Shift_R, KeyShift)
				KEY(XK_Control_L, KeyControl)
				KEY(XK_Control_R, KeyControl)
				KEY(XK_Alt_L, KeyAlt)
				KEY(XK_Alt_R, KeyAlt)
				KEY(XK_Delete, KeyDelete)
				KEY(XK_a, KeyA)
				KEY(XK_b, KeyB)
				KEY(XK_c, KeyC)
				KEY(XK_d, KeyD)
				KEY(XK_e, KeyE)
				KEY(XK_f, KeyF)
				KEY(XK_g, KeyG)
				KEY(XK_h, KeyH)
				KEY(XK_i, KeyI)
				KEY(XK_j, KeyJ)
				KEY(XK_k, KeyK)
				KEY(XK_l, KeyL)
				KEY(XK_m, KeyM)
				KEY(XK_n, KeyN)
				KEY(XK_o, KeyO)
				KEY(XK_p, KeyP)
				KEY(XK_q, KeyQ)
				KEY(XK_r, KeyR)
				KEY(XK_s, KeyS)
				KEY(XK_t, KeyT)
				KEY(XK_u, KeyU)
				KEY(XK_v, KeyV)
				KEY(XK_w, KeyW)
				KEY(XK_x, KeyX)
				KEY(XK_y, KeyY)
				KEY(XK_z, KeyZ)
				KEY(XK_1, Key1)
				KEY(XK_2, Key2)
				KEY(XK_3, Key3)
				KEY(XK_4, Key4)
				KEY(XK_5, Key5)
				KEY(XK_6, Key6)
				KEY(XK_7, Key7)
				KEY(XK_8, Key8)
				KEY(XK_9, Key9)
				KEY(XK_0, Key0)
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
			case Button2:
				Kore::Mouse::the()->_press(windowId, 2, button->x, button->y);
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
			case Button2:
				Kore::Mouse::the()->_release(windowId, 2, button->x, button->y);
				break;
			case Button3:
				Kore::Mouse::the()->_release(windowId, 1, button->x, button->y);
				break;
			// Button4 and Button5 provide mouse wheel events because why not
			case Button4:
				Kore::Mouse::the()->_scroll(windowId, -1);
				break;
			case Button5:
				Kore::Mouse::the()->_scroll(windowId, 1);
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
		case ConfigureNotify: {
            int windowId = windowimpl::idFromWindow(event.xconfigure.window);
            windowimpl::windows[windowId]->width = event.xconfigure.width;
            windowimpl::windows[windowId]->height = event.xconfigure.height;
			glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
            break;
		}
		case ClientMessage: {
			if (event.xclient.message_type == XdndDrop) {
				XdndSourceWindow = event.xclient.data.l[0];
				XConvertSelection(dpy, XdndSelection, XA_STRING, XdndPrimary, win, event.xclient.data.l[2]);
			}
			break;
		}
		case SelectionNotify: {
			Atom target = event.xselection.target;
			if (event.xselection.selection == clipboard) {
				int a = 3;
				++a;
			}
			else if (event.xselection.property) {
				char* result;
				unsigned long ressize, restail;
				int resbits;
				XGetWindowProperty(dpy, win, xseldata, 0, LONG_MAX / 4, False, AnyPropertyType,
				                   &utf8, &resbits, &ressize, &restail, (unsigned char**)&result);
				Kore::System::pasteCallback(result);
				XFree(result);
			}
			else if (target == XA_STRING) {
				Atom type;
				int format;
				unsigned long numItems;
				unsigned long bytesAfter = 1;
				unsigned char* data = 0;
				int readBytes = 1024;
				while (bytesAfter != 0) {
					if (data != 0) XFree(data);
					XGetWindowProperty(dpy, win, XdndPrimary, 0, readBytes, false, AnyPropertyType, &type, &format, &numItems, &bytesAfter, &data);
					readBytes *= 2;
				}
				size_t len = numItems * format / 8 - 1; // Strip new line at the end
				wchar_t filePath[len + 1];
				mbstowcs(filePath, (char*)data, len);
				filePath[len] = 0;
				Kore::System::dropFilesCallback(filePath + 7); // Strip file://

				XClientMessageEvent m;
				memset(&m, sizeof(m), 0);
				m.type = ClientMessage;
				m.display = dpy;
				m.window = XdndSourceWindow;
				m.message_type = XdndFinished;
				m.format = 32;
				m.data.l[0] = win;
				m.data.l[1] = 1;
				m.data.l[2] = XdndActionCopy;
				XSendEvent(dpy, XdndSourceWindow, false, NoEventMask, (XEvent*)&m);
				XSync(dpy, false);
			}
		}
		case Expose:
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
			case 111:
				Kore::Keyboard::the()->_keydown(Kore::Key_Up, ' ');
				break;
			case 116:
				Kore::Keyboard::the()->_keydown(Kore::Key_Down, ' ');
				break;
			case 113:
				Kore::Keyboard::the()->_keydown(Kore::Key_Left, ' ');
				break;
			case 114:
				Kore::Keyboard::the()->_keydown(Kore::Key_Right, ' ');
				break;
			}
			break;
		}
		case XCB_KEY_RELEASE: {
			const xcb_key_release_event_t* key = (const xcb_key_release_event_t*)event;
			if (key->detail == 0x9) exit(0);
			switch (key->detail) {
			case 111:
				Kore::Keyboard::the()->_keyup(Kore::Key_Up, ' ');
				break;
			case 116:
				Kore::Keyboard::the()->_keyup(Kore::Key_Down, ' ');
				break;
			case 113:
				Kore::Keyboard::the()->_keyup(Kore::Key_Left, ' ');
				break;
			case 114:
				Kore::Keyboard::the()->_keyup(Kore::Key_Right, ' ');
				break;
			}
			break;
		}
		case XCB_DESTROY_NOTIFY:
			exit(0);
			break;
		case XCB_CONFIGURE_NOTIFY: {
			const xcb_configure_notify_event_t* cfg = (const xcb_configure_notify_event_t*)event;
			// if ((demo->width != cfg->width) || (demo->height != cfg->height)) {
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
	Kore::updateHIDGamepads();
	return true;
}

const char* Kore::System::systemId() {
	return "Linux";
}

void Kore::System::makeCurrent(int contextId) {
	if (currentDeviceId == contextId) {
		return;
	}
	currentDeviceId = contextId;
#ifdef KORE_OPENGL
	glXMakeCurrent(dpy, windowimpl::windows[contextId]->handle, windowimpl::windows[contextId]->context);
#endif
}

void Kore::Graphics4::clearCurrent() {}

void Kore::System::clearCurrent() {
	currentDeviceId = -1;
	Graphics4::clearCurrent();
}

void Kore::System::swapBuffers(int contextId) {
#ifdef KORE_OPENGL
	glXSwapBuffers(dpy, windowimpl::windows[contextId]->handle);
#endif
}

void Kore::System::destroyWindow(int id) {
	// TODO (DK) implement me
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {}

void Kore::System::setTitle(const char* title) {}

void Kore::System::setKeepScreenOn(bool on) {}

void Kore::System::showWindow() {}

void Kore::System::showKeyboard() {}

void Kore::System::hideKeyboard() {}

void Kore::System::loadURL(const char* url) {}

int Kore::System::desktopWidth() {
#ifdef KORE_OPENGL
	return XWidthOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
	return 1920;
#endif
}

int Kore::System::desktopHeight() {
#ifdef KORE_OPENGL
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
	const char* videoFormats[] = {"ogv", nullptr};
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
	Kore::initHIDGamepads();
	kore(argc, argv);
}
