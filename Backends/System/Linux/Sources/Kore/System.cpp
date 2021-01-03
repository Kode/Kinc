#include "pch.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/input/gamepad.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/input/pen.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "Input/Gamepad.h"

#include <kinc/display.h>
#include <Kore/Linux.h>

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include "WindowData.h"

#ifdef KORE_OPENGL
#include <GL/gl.h>
#include <GL/glx.h>
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#endif

_XDisplay* Kore::Linux::display = nullptr;

namespace {
	struct MwmHints {
		// These correspond to XmRInt resources. (VendorSE.c)
		int flags;
		int functions;
		int decorations;
		int input_mode;
		int status;
	};
#define MWM_HINTS_DECORATIONS (1L << 1)
#ifdef KORE_OPENGL
	GLboolean doubleBuffer = GL_TRUE;
#endif
	Window win;
	Atom XdndDrop;
	Atom XdndEnter;
	Atom XdndTextUriList;
	Atom XdndStatus;
	Atom XdndActionCopy;
	Atom XdndSelection;
	Atom clipboard;
	Atom utf8;
	Atom xseldata;
	Atom targets;
	Atom multiple;
	Atom textplain;
	Window XdndSourceWindow = None;
	uint32_t penMotionEvent;
	uint32_t penMaxPressure = 2048;
	float penPressureLast = 0.0;
	XID penDevice;
	uint32_t eraserMotionEvent;
	uint32_t eraserMaxPressure = 2048;
	float eraserPressureLast = 0.0;
	XID eraserDevice;
	bool keyPressed[256];
	char clipboardString[4096];

	void fatalError(const char* message) {
		printf("main: %s\n", message);
		exit(1);
	}

	Atom wmDeleteMessage;
}

#define MAXIMUM_WINDOWS 16
extern Kore::WindowData kinc_internal_windows[MAXIMUM_WINDOWS];

namespace windowimpl {
	int windowCounter = -1;

	int idFromWindow(Window window) {
		for (int windowIndex = 0; windowIndex < MAXIMUM_WINDOWS; ++windowIndex) {
			if (kinc_internal_windows[windowIndex].handle == window) {
				return windowIndex;
			}
		}

		return -1;
	}
}

static char nameClass[256];
static const char *nameClassAddendum = "_KincApplication";

int createWindow(const char* title, int x, int y, int width, int height, kinc_window_mode_t windowMode, int targetDisplay, int depthBufferBits,
				 int stencilBufferBits, int antialiasingSamples) {
	strncpy(nameClass, kinc_application_name(), sizeof(nameClass) - strlen(nameClassAddendum) - 1);
	strcat(nameClass, nameClassAddendum);

	int wcounter = windowimpl::windowCounter + 1;

	if (Kore::Linux::display == nullptr) {
		Kore::Linux::display = XOpenDisplay(nullptr);
	}

	if (Kore::Linux::display == nullptr) {
		fatalError("could not open display");
	}

	XkbSetDetectableAutoRepeat(Kore::Linux::display, True, NULL);

	XSetWindowAttributes swa;
	swa.border_pixel = 0;
	swa.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;
	Colormap cmap;

#ifdef KORE_OPENGL
	XVisualInfo* vi;
	GLXContext cx;
	int dummy;

	if (!glXQueryExtension(Kore::Linux::display, &dummy, &dummy)) {
		fatalError("X server has no OpenGL GLX extension");
	}

	int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, depthBufferBits, GLX_STENCIL_SIZE, stencilBufferBits, None};
	int dblBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, depthBufferBits, GLX_STENCIL_SIZE, stencilBufferBits, GLX_DOUBLEBUFFER, None};

	vi = nullptr;

	int attribs[] = {
		GLX_X_RENDERABLE    , True,
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
		None
	};

	GLXFBConfig fbconfig = 0;
	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(Kore::Linux::display, DefaultScreen(Kore::Linux::display), attribs, &fbcount);
	if (fbc) {
		if (fbcount >= 1) {
			fbconfig = fbc[0];
		}
		XFree(fbc);
	}

	if (fbconfig) {
		vi = glXGetVisualFromFBConfig(Kore::Linux::display, fbconfig);
	}

	if (vi == nullptr) {
		vi = glXChooseVisual(Kore::Linux::display, DefaultScreen(Kore::Linux::display), dblBuf);
	}

	if (vi == nullptr) {
		vi = glXChooseVisual(Kore::Linux::display, DefaultScreen(Kore::Linux::display), snglBuf);

		if (vi == nullptr) {
			fatalError("no RGB visual with valid depth/stencil buffer");
		}

		doubleBuffer = GL_FALSE;
	}
	// if(vi->class != TrueColor)
	//  fatalError("TrueColor visual required for this program");

	// TODO (DK)
	//  -context sharing doesn't seem to work in virtual box?
	//      -main screen flickers
	//      -sprite in subscreens is black
	cx = NULL;
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
	if (glXCreateContextAttribsARB) {
		int contextAttribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			GLX_CONTEXT_PROFILE_MASK_ARB , GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			None
		};
		if (fbconfig) {
			cx = glXCreateContextAttribsARB(Kore::Linux::display, fbconfig,
			                                wcounter == 0 ? None : kinc_internal_windows[0].context, GL_TRUE,
			                                contextAttribs);
		}
	}

	if (cx == NULL) {
		cx = glXCreateContext(Kore::Linux::display, vi, wcounter == 0 ? None : kinc_internal_windows[0].context, /* direct rendering if possible */ GL_TRUE);
	}

	if (cx == NULL) {
		fatalError("could not create rendering context");
	}

	cmap = XCreateColormap(Kore::Linux::display, RootWindow(Kore::Linux::display, vi->screen), vi->visual, AllocNone);
	swa.colormap = cmap;
	win = XCreateWindow(Kore::Linux::display, RootWindow(Kore::Linux::display, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
							   CWBorderPixel | CWColormap | CWEventMask, &swa);
#else
    int screen = DefaultScreen(Kore::Linux::display);
    Visual* visual = DefaultVisual(Kore::Linux::display, screen);
    int depth  = DefaultDepth(Kore::Linux::display, screen);
    cmap = XCreateColormap(Kore::Linux::display, RootWindow(Kore::Linux::display, screen), visual, AllocNone);
	swa.colormap = cmap;
	win = XCreateWindow(Kore::Linux::display, RootWindow(Kore::Linux::display, DefaultScreen(Kore::Linux::display)), 0, 0, width, height, 0, depth, InputOutput, visual,
						CWBorderPixel | CWColormap | CWEventMask, &swa);
#endif

	XSetStandardProperties(Kore::Linux::display, win, title, "main", None, NULL, 0, NULL);

	Atom wmClassAtom = XInternAtom(Kore::Linux::display, "WM_CLASS", 0);
	XChangeProperty(Kore::Linux::display, win, wmClassAtom, XA_STRING, 8, PropModeReplace, (unsigned char*)nameClass, strlen(nameClass));

	switch (windowMode) {
	case KINC_WINDOW_MODE_WINDOW:
		break;
	case KINC_WINDOW_MODE_FULLSCREEN: // fall through
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN: {
		Atom awmHints = XInternAtom(Kore::Linux::display, "_MOTIF_WM_HINTS", 0);
		MwmHints hints;
		hints.flags = MWM_HINTS_DECORATIONS;
		hints.decorations = 0;

		XChangeProperty(Kore::Linux::display, win, awmHints, awmHints, 32, PropModeReplace, (unsigned char*)&hints, 5);
	}
	}

#ifdef KORE_OPENGL
	glXMakeCurrent(Kore::Linux::display, win, cx);
#endif

	int display = targetDisplay == -1 ? kinc_primary_display() : targetDisplay;
    kinc_display_mode_t deviceInfo = kinc_display_current_mode(display);

	int dstx = deviceInfo.x;
	int dsty = deviceInfo.y;

	switch (windowMode) {
	default: {
		int dw = deviceInfo.width;
		int dh = deviceInfo.height;
		dstx += x < 0 ? (dw - width) / 2 : x;
		dsty += y < 0 ? (dh - height) / 2 : y;
	} break;
	}

	XMapWindow(Kore::Linux::display, win);
	XMoveWindow(Kore::Linux::display, win, dstx, dsty);
	// Scheduler::addFrameTask(HandleMessages, 1001);

	// Drag and drop
	Atom XdndAware = XInternAtom(Kore::Linux::display, "XdndAware", 0);
	Atom XdndVersion = 5;
	XChangeProperty(Kore::Linux::display, win, XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&XdndVersion, 1);
	XdndDrop = XInternAtom(Kore::Linux::display, "XdndDrop", 0);
	XdndEnter = XInternAtom(Kore::Linux::display, "XdndEnter", 0);
	XdndTextUriList = XInternAtom(Kore::Linux::display, "text/uri-list", 0);
	XdndStatus = XInternAtom(Kore::Linux::display, "XdndStatus", 0);
	XdndActionCopy = XInternAtom(Kore::Linux::display, "XdndActionCopy", 0);
	XdndSelection = XInternAtom(Kore::Linux::display, "XdndSelection", 0);
	clipboard = XInternAtom(Kore::Linux::display, "CLIPBOARD", 0);
	utf8 = XInternAtom(Kore::Linux::display, "UTF8_STRING", 0);
	xseldata = XInternAtom(Kore::Linux::display, "XSEL_DATA", 0);
	targets = XInternAtom(Kore::Linux::display, "TARGETS", 0);
	multiple = XInternAtom(Kore::Linux::display, "MULTIPLE", 0);
	textplain = XInternAtom(Kore::Linux::display, "text/plain;charset=utf-8", 0);

	int count;
	XDeviceInfoPtr devices = (XDeviceInfoPtr)XListInputDevices(Kore::Linux::display, &count);
	for (int i = 0; i < count; i++) {
		if (strstr(devices[i].name, "stylus")) {
			XDevice* device = XOpenDevice(Kore::Linux::display, devices[i].id);
			penDevice = devices[i].id;
			XAnyClassPtr c = devices[i].inputclassinfo;
			for (int j = 0; j < devices[i].num_classes; j++) {
				if (c->c_class == ValuatorClass) {
					XValuatorInfo* info = (XValuatorInfo*)c;
					if (info->num_axes > 2) {
						penMaxPressure = info->axes[2].max_value;
					}
					XEventClass eventClass;
					DeviceMotionNotify(device, penMotionEvent, eventClass);
					XSelectExtensionEvent(Kore::Linux::display, win, &eventClass, 1);
					break;
				}
				c = (XAnyClassPtr)((uint8_t*)c + c->length);
			}
		}
		if (strstr(devices[i].name, "eraser")) {
			XDevice* device = XOpenDevice(Kore::Linux::display, devices[i].id);
			eraserDevice = devices[i].id;
			XAnyClassPtr c = devices[i].inputclassinfo;
			for (int j = 0; j < devices[i].num_classes; j++) {
				if (c->c_class == ValuatorClass) {
					XValuatorInfo* info = (XValuatorInfo*)c;
					if (info->num_axes > 2) {
						eraserMaxPressure = info->axes[2].max_value;
					}
					XEventClass eventClass;
					DeviceMotionNotify(device, eraserMotionEvent, eventClass);
					XSelectExtensionEvent(Kore::Linux::display, win, &eventClass, 1);
					break;
				}
				c = (XAnyClassPtr)((uint8_t*)c + c->length);
			}
		}
	}

    kinc_internal_windows[wcounter].width = width;
    kinc_internal_windows[wcounter].height = height;
    kinc_internal_windows[wcounter].handle = win;
#ifdef KORE_OPENGL
    kinc_internal_windows[wcounter].context = cx;
#endif

	if (windowMode == KINC_WINDOW_MODE_FULLSCREEN || windowMode == KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
		Kore::Linux::fullscreen(win, true);
        kinc_internal_windows[wcounter].mode = windowMode;
	}
	else {
        kinc_internal_windows[wcounter].mode = 0;
	}

	wmDeleteMessage = XInternAtom(Kore::Linux::display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(Kore::Linux::display, win, &wmDeleteMessage, 1);

	return windowimpl::windowCounter = wcounter;
}

namespace Kore {
	namespace System {
		int initWindow(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
			char buffer[1024] = {0};
			strcpy(buffer, kinc_application_name());
			if (win->title != nullptr) {
				strcpy(buffer, win->title);
			}

			int id = createWindow(buffer, win->x, win->y, win->width, win->height, win->mode, win->display_index,
								  frame->depth_bits, frame->stencil_bits, frame->samples_per_pixel);
			kinc_g4_init(id, frame->depth_bits, frame->stencil_bits, true);
			return id;
		}

		int currentDeviceId = -1;

		int currentDevice() {
			return currentDeviceId;
		}

		void setCurrentDevice(int id) {
			currentDeviceId = id;
		}
	}
}
bool kinc_internal_handle_messages() {
	static bool controlDown = false;
	while (XPending(Kore::Linux::display) > 0) {
		XEvent event;
		XNextEvent(Kore::Linux::display, &event);

		if (event.type == penMotionEvent) {
			XDeviceMotionEvent* motion = (XDeviceMotionEvent*)(&event);
			if (motion->deviceid == penDevice) {
				int windowId = windowimpl::idFromWindow(motion->window);
				float p = (float)motion->axis_data[2] / (float)penMaxPressure;
				if (p > 0 && penPressureLast == 0) {
					kinc_internal_pen_trigger_press(windowId, motion->x, motion->y, p);
				}
				else if (p == 0 && penPressureLast > 0) {
					kinc_internal_pen_trigger_release(windowId, motion->x, motion->y, p);
				}
				else if (p > 0) {
					kinc_internal_pen_trigger_move(windowId, motion->x, motion->y, p);
				}
				penPressureLast = p;
			}
		}
		if (event.type == eraserMotionEvent) {
			XDeviceMotionEvent* motion = (XDeviceMotionEvent*)(&event);
			if (motion->deviceid == eraserDevice) {
				int windowId = windowimpl::idFromWindow(motion->window);
				float p = (float)motion->axis_data[2] / (float)eraserMaxPressure;
				if (p > 0 && eraserPressureLast == 0) {
					kinc_internal_eraser_trigger_press(windowId, motion->x, motion->y, p);
				}
				else if (p == 0 && eraserPressureLast > 0) {
					kinc_internal_eraser_trigger_release(windowId, motion->x, motion->y, p);
				}
				else if (p > 0) {
					kinc_internal_eraser_trigger_move(windowId, motion->x, motion->y, p);
				}
				eraserPressureLast = p;
			}
		}

		switch (event.type) {
		case KeyPress: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);

			if (buffer[0] >= 32 && buffer[0] <= 126) {
                kinc_internal_keyboard_trigger_key_press((wchar_t)buffer[0]);
			}

#define KEY(xkey, korekey)                                          \
	case xkey:                                                      \
		if (!keyPressed[korekey]) {                           \
			keyPressed[korekey] = true;                       \
			kinc_internal_keyboard_trigger_key_down(korekey);   \
		}                                                           \
		break;

			if (keysym == XK_Control_L || keysym == XK_Control_R) {
				controlDown = true;
			}
			else if (controlDown && (keysym == XK_v || keysym == XK_V)) {
				XConvertSelection(Kore::Linux::display, clipboard, utf8, xseldata, win, CurrentTime);
			}
			else if (controlDown && (keysym == XK_c || keysym == XK_C)) {
				XSetSelectionOwner(Kore::Linux::display, clipboard, win, CurrentTime);
				char *text = kinc_internal_copy_callback();
				if (text != nullptr) strcpy(clipboardString, text);
			}
			else if (controlDown && (keysym == XK_x || keysym == XK_X)) {
				XSetSelectionOwner(Kore::Linux::display, clipboard, win, CurrentTime);
				char *text = kinc_internal_cut_callback();
				if (text != nullptr) strcpy(clipboardString, text);
			}

			KeySym ksKey = XkbKeycodeToKeysym(Kore::Linux::display, event.xkey.keycode, 0, 0);

			switch (ksKey) {
				KEY(XK_Right, KINC_KEY_RIGHT)
				KEY(XK_Left, KINC_KEY_LEFT)
				KEY(XK_Up, KINC_KEY_UP)
				KEY(XK_Down, KINC_KEY_DOWN)
				KEY(XK_space, KINC_KEY_SPACE)
				KEY(XK_BackSpace, KINC_KEY_BACKSPACE)
				KEY(XK_Tab, KINC_KEY_TAB)
				KEY(XK_Return, KINC_KEY_RETURN)
				KEY(XK_Shift_L, KINC_KEY_SHIFT)
				KEY(XK_Shift_R, KINC_KEY_SHIFT)
				KEY(XK_Control_L, KINC_KEY_CONTROL)
				KEY(XK_Control_R, KINC_KEY_CONTROL)
				KEY(XK_Alt_L, KINC_KEY_ALT)
				KEY(XK_Alt_R, KINC_KEY_ALT)
				KEY(XK_Delete, KINC_KEY_DELETE)
				KEY(XK_comma, KINC_KEY_COMMA)
				KEY(XK_period, KINC_KEY_PERIOD)
				KEY(XK_bracketleft, KINC_KEY_OPEN_BRACKET)
				KEY(XK_bracketright, KINC_KEY_CLOSE_BRACKET)
				KEY(XK_backslash, KINC_KEY_BACK_SLASH)
				KEY(XK_apostrophe, KINC_KEY_QUOTE)
				KEY(XK_semicolon, KINC_KEY_SEMICOLON)
				KEY(XK_minus, KINC_KEY_HYPHEN_MINUS)
				KEY(XK_slash, KINC_KEY_SLASH)
				KEY(XK_less, KINC_KEY_LESS_THAN)
				KEY(XK_equal, KINC_KEY_EQUALS)
				KEY(XK_quoteleft, KINC_KEY_BACK_QUOTE)
				KEY(XK_Pause, KINC_KEY_PAUSE)
				KEY(XK_Scroll_Lock, KINC_KEY_SCROLL_LOCK)
				KEY(XK_Home, KINC_KEY_HOME)
				KEY(XK_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_End, KINC_KEY_END)
				KEY(XK_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Enter, KINC_KEY_RETURN)
				KEY(XK_KP_Multiply, KINC_KEY_MULTIPLY)
				KEY(XK_KP_Add, KINC_KEY_ADD)
				KEY(XK_KP_Subtract, KINC_KEY_SUBTRACT)
				KEY(XK_KP_Decimal, KINC_KEY_DECIMAL)
				KEY(XK_KP_Divide, KINC_KEY_DIVIDE)
				KEY(XK_KP_0, KINC_KEY_0)
				KEY(XK_KP_1, KINC_KEY_1)
				KEY(XK_KP_2, KINC_KEY_2)
				KEY(XK_KP_3, KINC_KEY_3)
				KEY(XK_KP_4, KINC_KEY_4)
				KEY(XK_KP_5, KINC_KEY_5)
				KEY(XK_KP_6, KINC_KEY_6)
				KEY(XK_KP_7, KINC_KEY_7)
				KEY(XK_KP_8, KINC_KEY_8)
				KEY(XK_KP_9, KINC_KEY_9)
				KEY(XK_KP_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Delete, KINC_KEY_DELETE)
				KEY(XK_KP_End, KINC_KEY_END)
				KEY(XK_KP_Home, KINC_KEY_HOME)
				KEY(XK_KP_Left, KINC_KEY_LEFT)
				KEY(XK_KP_Up, KINC_KEY_UP)
				KEY(XK_KP_Right, KINC_KEY_RIGHT)
				KEY(XK_KP_Down, KINC_KEY_DOWN)
				KEY(XK_KP_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_KP_Page_Down, KINC_KEY_PAGE_DOWN)
				KEY(XK_Menu, KINC_KEY_CONTEXT_MENU)
				KEY(XK_a, KINC_KEY_A)
				KEY(XK_b, KINC_KEY_B)
				KEY(XK_c, KINC_KEY_C)
				KEY(XK_d, KINC_KEY_D)
				KEY(XK_e, KINC_KEY_E)
				KEY(XK_f, KINC_KEY_F)
				KEY(XK_g, KINC_KEY_G)
				KEY(XK_h, KINC_KEY_H)
				KEY(XK_i, KINC_KEY_I)
				KEY(XK_j, KINC_KEY_J)
				KEY(XK_k, KINC_KEY_K)
				KEY(XK_l, KINC_KEY_L)
				KEY(XK_m, KINC_KEY_M)
				KEY(XK_n, KINC_KEY_N)
				KEY(XK_o, KINC_KEY_O)
				KEY(XK_p, KINC_KEY_P)
				KEY(XK_q, KINC_KEY_Q)
				KEY(XK_r, KINC_KEY_R)
				KEY(XK_s, KINC_KEY_S)
				KEY(XK_t, KINC_KEY_T)
				KEY(XK_u, KINC_KEY_U)
				KEY(XK_v, KINC_KEY_V)
				KEY(XK_w, KINC_KEY_W)
				KEY(XK_x, KINC_KEY_X)
				KEY(XK_y, KINC_KEY_Y)
				KEY(XK_z, KINC_KEY_Z)
				KEY(XK_1, KINC_KEY_1)
				KEY(XK_2, KINC_KEY_2)
				KEY(XK_3, KINC_KEY_3)
				KEY(XK_4, KINC_KEY_4)
				KEY(XK_5, KINC_KEY_5)
				KEY(XK_6, KINC_KEY_6)
				KEY(XK_7, KINC_KEY_7)
				KEY(XK_8, KINC_KEY_8)
				KEY(XK_9, KINC_KEY_9)
				KEY(XK_0, KINC_KEY_0)
				KEY(XK_Escape, KINC_KEY_ESCAPE)
				KEY(XK_F1, KINC_KEY_F1)
				KEY(XK_F2, KINC_KEY_F2)
				KEY(XK_F3, KINC_KEY_F3)
				KEY(XK_F4, KINC_KEY_F4)
				KEY(XK_F5, KINC_KEY_F5)
				KEY(XK_F6, KINC_KEY_F6)
				KEY(XK_F7, KINC_KEY_F7)
				KEY(XK_F8, KINC_KEY_F8)
				KEY(XK_F9, KINC_KEY_F9)
				KEY(XK_F10, KINC_KEY_F10)
				KEY(XK_F11, KINC_KEY_F11)
				KEY(XK_F12, KINC_KEY_F12)
			}
			break;
#undef KEY
		}
		case KeyRelease: {
			XKeyEvent* key = (XKeyEvent*)&event;
			KeySym keysym;
			char buffer[1];
			XLookupString(key, buffer, 1, &keysym, NULL);

#define KEY(xkey, korekey)                                      \
	case xkey:                                                  \
		kinc_internal_keyboard_trigger_key_up(korekey);     \
		keyPressed[korekey] = false;                      \
		break;

			if (keysym == XK_Control_L || keysym == XK_Control_R) {
				controlDown = false;
			}

			KeySym ksKey = XkbKeycodeToKeysym(Kore::Linux::display, event.xkey.keycode, 0, 0);

			switch (ksKey) {
				KEY(XK_Right, KINC_KEY_RIGHT)
				KEY(XK_Left, KINC_KEY_LEFT)
				KEY(XK_Up, KINC_KEY_UP)
				KEY(XK_Down, KINC_KEY_DOWN)
				KEY(XK_space, KINC_KEY_SPACE)
				KEY(XK_BackSpace, KINC_KEY_BACKSPACE)
				KEY(XK_Tab, KINC_KEY_TAB)
				KEY(XK_Return, KINC_KEY_RETURN)
				KEY(XK_Shift_L, KINC_KEY_SHIFT)
				KEY(XK_Shift_R, KINC_KEY_SHIFT)
				KEY(XK_Control_L, KINC_KEY_CONTROL)
				KEY(XK_Control_R, KINC_KEY_CONTROL)
				KEY(XK_Alt_L, KINC_KEY_ALT)
				KEY(XK_Alt_R, KINC_KEY_ALT)
				KEY(XK_Delete, KINC_KEY_DELETE)
				KEY(XK_comma, KINC_KEY_COMMA)
				KEY(XK_period, KINC_KEY_PERIOD)
				KEY(XK_bracketleft, KINC_KEY_OPEN_BRACKET)
				KEY(XK_bracketright, KINC_KEY_CLOSE_BRACKET)
				KEY(XK_backslash, KINC_KEY_BACK_SLASH)
				KEY(XK_apostrophe, KINC_KEY_QUOTE)
				KEY(XK_semicolon, KINC_KEY_SEMICOLON)
				KEY(XK_minus, KINC_KEY_HYPHEN_MINUS)
				KEY(XK_slash, KINC_KEY_SLASH)
				KEY(XK_less, KINC_KEY_LESS_THAN)
				KEY(XK_equal, KINC_KEY_EQUALS)
				KEY(XK_quoteleft, KINC_KEY_BACK_QUOTE)
				KEY(XK_Pause, KINC_KEY_PAUSE)
				KEY(XK_Scroll_Lock, KINC_KEY_SCROLL_LOCK)
				KEY(XK_Home, KINC_KEY_HOME)
				KEY(XK_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_End, KINC_KEY_END)
				KEY(XK_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Enter, KINC_KEY_RETURN)
				KEY(XK_KP_Multiply, KINC_KEY_MULTIPLY)
				KEY(XK_KP_Add, KINC_KEY_ADD)
				KEY(XK_KP_Subtract, KINC_KEY_SUBTRACT)
				KEY(XK_KP_Decimal, KINC_KEY_DECIMAL)
				KEY(XK_KP_Divide, KINC_KEY_DIVIDE)
				KEY(XK_KP_0, KINC_KEY_0)
				KEY(XK_KP_1, KINC_KEY_1)
				KEY(XK_KP_2, KINC_KEY_2)
				KEY(XK_KP_3, KINC_KEY_3)
				KEY(XK_KP_4, KINC_KEY_4)
				KEY(XK_KP_5, KINC_KEY_5)
				KEY(XK_KP_6, KINC_KEY_6)
				KEY(XK_KP_7, KINC_KEY_7)
				KEY(XK_KP_8, KINC_KEY_8)
				KEY(XK_KP_9, KINC_KEY_9)
				KEY(XK_KP_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Delete, KINC_KEY_DELETE)
				KEY(XK_KP_End, KINC_KEY_END)
				KEY(XK_KP_Home, KINC_KEY_HOME)
				KEY(XK_KP_Left, KINC_KEY_LEFT)
				KEY(XK_KP_Up, KINC_KEY_UP)
				KEY(XK_KP_Right, KINC_KEY_RIGHT)
				KEY(XK_KP_Down, KINC_KEY_DOWN)
				KEY(XK_KP_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_KP_Page_Down, KINC_KEY_PAGE_DOWN)
				KEY(XK_Menu, KINC_KEY_CONTEXT_MENU)
				KEY(XK_a, KINC_KEY_A)
				KEY(XK_b, KINC_KEY_B)
				KEY(XK_c, KINC_KEY_C)
				KEY(XK_d, KINC_KEY_D)
				KEY(XK_e, KINC_KEY_E)
				KEY(XK_f, KINC_KEY_F)
				KEY(XK_g, KINC_KEY_G)
				KEY(XK_h, KINC_KEY_H)
				KEY(XK_i, KINC_KEY_I)
				KEY(XK_j, KINC_KEY_J)
				KEY(XK_k, KINC_KEY_K)
				KEY(XK_l, KINC_KEY_L)
				KEY(XK_m, KINC_KEY_M)
				KEY(XK_n, KINC_KEY_N)
				KEY(XK_o, KINC_KEY_O)
				KEY(XK_p, KINC_KEY_P)
				KEY(XK_q, KINC_KEY_Q)
				KEY(XK_r, KINC_KEY_R)
				KEY(XK_s, KINC_KEY_S)
				KEY(XK_t, KINC_KEY_T)
				KEY(XK_u, KINC_KEY_U)
				KEY(XK_v, KINC_KEY_V)
				KEY(XK_w, KINC_KEY_W)
				KEY(XK_x, KINC_KEY_X)
				KEY(XK_y, KINC_KEY_Y)
				KEY(XK_z, KINC_KEY_Z)
				KEY(XK_1, KINC_KEY_1)
				KEY(XK_2, KINC_KEY_2)
				KEY(XK_3, KINC_KEY_3)
				KEY(XK_4, KINC_KEY_4)
				KEY(XK_5, KINC_KEY_5)
				KEY(XK_6, KINC_KEY_6)
				KEY(XK_7, KINC_KEY_7)
				KEY(XK_8, KINC_KEY_8)
				KEY(XK_9, KINC_KEY_9)
				KEY(XK_0, KINC_KEY_0)
				KEY(XK_Escape, KINC_KEY_ESCAPE)
				KEY(XK_F1, KINC_KEY_F1)
				KEY(XK_F2, KINC_KEY_F2)
				KEY(XK_F3, KINC_KEY_F3)
				KEY(XK_F4, KINC_KEY_F4)
				KEY(XK_F5, KINC_KEY_F5)
				KEY(XK_F6, KINC_KEY_F6)
				KEY(XK_F7, KINC_KEY_F7)
				KEY(XK_F8, KINC_KEY_F8)
				KEY(XK_F9, KINC_KEY_F9)
				KEY(XK_F10, KINC_KEY_F10)
				KEY(XK_F11, KINC_KEY_F11)
				KEY(XK_F12, KINC_KEY_F12)
			}
			break;
#undef KEY
		}
		case ButtonPress: {
			XButtonEvent* button = (XButtonEvent*)&event;
			int windowId = windowimpl::idFromWindow(button->window);

			switch (button->button) {
			case Button1:
                kinc_internal_mouse_trigger_press(windowId, 0, button->x, button->y);
				break;
			case Button2:
                kinc_internal_mouse_trigger_press(windowId, 2, button->x, button->y);
				break;
			case Button3:
                kinc_internal_mouse_trigger_press(windowId, 1, button->x, button->y);
				break;
			}
			break;
		}
		case ButtonRelease: {
			XButtonEvent* button = (XButtonEvent*)&event;
			int windowId = windowimpl::idFromWindow(button->window);

			switch (button->button) {
			case Button1:
                kinc_internal_mouse_trigger_release(windowId, 0, button->x, button->y);
				break;
			case Button2:
                kinc_internal_mouse_trigger_release(windowId, 2, button->x, button->y);
				break;
			case Button3:
                kinc_internal_mouse_trigger_release(windowId, 1, button->x, button->y);
				break;
			// Button4 and Button5 provide mouse wheel events because why not
			case Button4:
                kinc_internal_mouse_trigger_scroll(windowId, -1);
				break;
			case Button5:
                kinc_internal_mouse_trigger_scroll(windowId, 1);
				break;
			}
			break;
		}
		case MotionNotify: {
			XMotionEvent* motion = (XMotionEvent*)&event;
			int windowId = windowimpl::idFromWindow(motion->window);
            kinc_internal_mouse_trigger_move(windowId, motion->x, motion->y);
			break;
		}
		case ConfigureNotify: {
			int windowId = windowimpl::idFromWindow(event.xconfigure.window);
			if (event.xconfigure.width != kinc_internal_windows[windowId].width || event.xconfigure.height != kinc_internal_windows[windowId].height) {
				kinc_internal_windows[windowId].width = event.xconfigure.width;
				kinc_internal_windows[windowId].height = event.xconfigure.height;
				kinc_internal_call_resize_callback(windowId, event.xconfigure.width, event.xconfigure.height);
#ifdef KORE_OPENGL
				glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
#endif
			}
			break;
		}
		case ClientMessage: {
			if (event.xclient.message_type == XdndEnter) {
				XdndSourceWindow = event.xclient.data.l[0];
				XEvent m;
				memset(&m, 0, sizeof(m));
				m.type = ClientMessage;
				m.xclient.window = event.xclient.data.l[0];
				m.xclient.message_type = XdndStatus;
				m.xclient.format = 32;
				m.xclient.data.l[0] = win;
				m.xclient.data.l[2] = 0;
				m.xclient.data.l[3] = 0;
				m.xclient.data.l[1] = 1;
				m.xclient.data.l[4] = XdndActionCopy;
				XSendEvent(Kore::Linux::display, XdndSourceWindow, false, NoEventMask, (XEvent*)&m);
				XFlush(Kore::Linux::display);
			}
			else if (event.xclient.message_type == XdndDrop) {
				XConvertSelection(Kore::Linux::display, XdndSelection, XdndTextUriList, XdndSelection, win, event.xclient.data.l[2]);
			}
			else if (event.xclient.data.l[0] == wmDeleteMessage) {
                kinc_stop();
			}
			break;
		}
		case SelectionNotify: {
			if (event.xselection.selection == clipboard) {
				char* result;
				unsigned long ressize, restail;
				int resbits;
				XGetWindowProperty(Kore::Linux::display, win, xseldata, 0, LONG_MAX / 4, False, AnyPropertyType,
								   &utf8, &resbits, &ressize, &restail, (unsigned char**)&result);
				kinc_internal_paste_callback(result);
				XFree(result);
			}
			else if (event.xselection.property == XdndSelection) {
				Atom type;
				int format;
				unsigned long numItems;
				unsigned long bytesAfter = 1;
				unsigned char* data = 0;
				XGetWindowProperty(Kore::Linux::display, event.xselection.requestor, event.xselection.property, 0, LONG_MAX, False, event.xselection.target, &type, &format, &numItems, &bytesAfter, &data);
				size_t len = numItems * format / 8 - 1; // Strip new line at the end
				wchar_t filePath[len + 1];
				mbstowcs(filePath, (char*)data, len);
				XFree(data);
				filePath[len] = 0;
                kinc_internal_drop_files_callback(filePath + 7); // Strip file://
			}
			break;
		}
		case SelectionRequest: {
			if (event.xselectionrequest.target == targets) {
				XEvent send;
				send.xselection.type = SelectionNotify;
				send.xselection.requestor = event.xselectionrequest.requestor;
				send.xselection.selection = event.xselectionrequest.selection;
				send.xselection.target = event.xselectionrequest.target;
				send.xselection.property = event.xselectionrequest.property;
				send.xselection.time = event.xselectionrequest.time;
				Atom available[] = { targets, multiple, textplain, utf8 };
				XChangeProperty(Kore::Linux::display, send.xselection.requestor, send.xselection.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)&available[0], 4);
				XSendEvent(Kore::Linux::display, send.xselection.requestor, True, 0, &send);
			}
			if (event.xselectionrequest.target == textplain || event.xselectionrequest.target == utf8) {
				XEvent send;
				send.xselection.type = SelectionNotify;
				send.xselection.requestor = event.xselectionrequest.requestor;
				send.xselection.selection = event.xselectionrequest.selection;
				send.xselection.target = event.xselectionrequest.target;
				send.xselection.property = event.xselectionrequest.property;
				send.xselection.time = event.xselectionrequest.time;
				XChangeProperty(Kore::Linux::display, send.xselection.requestor, send.xselection.property, send.xselection.target, 8, PropModeReplace,
					(const unsigned char*)clipboardString, strlen(clipboardString));
				XSendEvent(Kore::Linux::display, send.xselection.requestor, True, 0, &send);
			}
			break;
		}
		case Expose:
			break;
		case FocusIn: {
			kinc_internal_foreground_callback();
			break;
		}
		case FocusOut: {
			kinc_internal_background_callback();
			break;
		}
		}
	}
	Kore::updateHIDGamepads();
	return true;
}

const char* kinc_system_id() {
	return "Linux";
}

void swapLinuxBuffers(int window) {
#ifdef KORE_OPENGL
	glXSwapBuffers(Kore::Linux::display, kinc_internal_windows[window].handle);
#endif
}

void kinc_set_keep_screen_on(bool on) {}

void kinc_keyboard_show() {}

void kinc_keyboard_hide() {}

bool kinc_keyboard_active() {
	return true;
}

void kinc_load_url(const char* url) {}

void kinc_vibrate(int ms) {}

const char* kinc_language() {
	return "en";
}

namespace {
	char save[2000];
	bool saveInitialized = false;
}

const char* kinc_internal_save_path() {
	if (!saveInitialized) {
		const char *homedir;

		if ((homedir = getenv("HOME")) == NULL) {
			homedir = getpwuid(getuid())->pw_dir;
		}

		strcpy(save, homedir);
		strcat(save, "/.");
		strcat(save, kinc_application_name());
		strcat(save, "/");

		int res = mkdir(save, 0700);

		saveInitialized = true;
	}
	return save;
}

namespace {
	const char* videoFormats[] = {"ogv", nullptr};
}

const char** kinc_video_formats() {
	return ::videoFormats;
}

#include <sys/time.h>
#include <time.h>

double kinc_frequency(void) {
	return 1000000.0;
}

static timeval start;

kinc_ticks_t kinc_timestamp(void) {
	timeval now;
	gettimeofday(&now, NULL);
	now.tv_sec -= start.tv_sec;
	now.tv_usec -= start.tv_usec;
	return static_cast<kinc_ticks_t>(now.tv_sec) * 1000000 + static_cast<kinc_ticks_t>(now.tv_usec);
}

void kinc_login() {

}

void kinc_unlock_achievement(int id) {

}

int kinc_init(const char* name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	for (int i = 0; i < 256; ++i) keyPressed[i] = false;
	Kore::initHIDGamepads();

	gettimeofday(&start, NULL);
	kinc_display_init();

	kinc_set_application_name(name);
	//System::_init(name, width, height, &win, &frame);
    kinc_window_options_t defaultWin;
    if (win == NULL) {
        kinc_internal_init_window_options(&defaultWin);
        win = &defaultWin;
    }
    kinc_framebuffer_options_t defaultFrame;
    if (frame == NULL) {
        kinc_internal_init_framebuffer_options(&defaultFrame);
        frame = &defaultFrame;
    }
    win->width = width;
    win->height = height;
    if (win->title == NULL) {
		win->title = name;
	}
	int window = Kore::System::initWindow(win, frame);
	return window;
}

void kinc_internal_shutdown() {

}

#ifndef KINC_NO_MAIN
int main(int argc, char** argv) {
	kickstart(argc, argv);
}
#endif

void kinc_copy_to_clipboard(const char* text) {
	strcpy(clipboardString, text);
}
