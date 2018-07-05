#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Window.h>
#include <Kore/Graphics4/Graphics.h>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>

#undef CreateWindow

using namespace Kore;

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void initializeDirectInput();
void loadXInput();

namespace {
	const int maximumWindows = 10;
	class WindowsWindow : public Window {
	public:
		HWND handle;
		bool mouseInside;

		WindowsWindow() : handle(nullptr), mouseInside(false) { }
	};

	WindowsWindow windows[maximumWindows];
	int windowCounter = -1;
	
	#ifdef KORE_OCULUS
	const wchar_t* windowClassName = L"ORT";
#else
	const wchar_t* windowClassName = L"KoreWindow";
#endif

	void registerWindowClass(HINSTANCE hInstance, const wchar_t* className) {
		WNDCLASSEXW wc = {sizeof(WNDCLASSEXA),
		                  CS_OWNDC /*CS_CLASSDC*/,
		                  MsgProc,
		                  0L,
		                  0L,
		                  hInstance,
		                  LoadIcon(hInstance, MAKEINTRESOURCE(107)),
		                  nullptr /*LoadCursor(0, IDC_ARROW)*/,
		                  0,
		                  0,
		                  className,
		                  0};
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		RegisterClassEx(&wc);
	}

	bool deviceModeChanged = false;
	DEVMODE startDeviceMode;
}

int idFromHWND(HWND handle) {
	for (int i = 0; i < maximumWindows; ++i) {
		if (windows[i].handle == handle) {
			return i;
		}
	}
	return -1;
}

int Window::count() {
	return windowCounter;
}

int Window::width() {
	RECT rect;
	GetClientRect((HWND)handle(), &rect);
	return rect.right;
}

int Window::height() {
	RECT rect;
	GetClientRect((HWND)handle(), &rect);
	return rect.bottom;
}

static int createWindow(const wchar_t* title, int x, int y, int width, int height, Kore::WindowMode windowMode, int targetDisplay) {
	++windowCounter;

	HINSTANCE inst = GetModuleHandle(nullptr);
#ifdef KORE_OCULUS
	::registerWindowClass(inst, windowClassName);
	//::windows[0] = new W32KoreWindow((HWND)VrInterface::Init(inst));
	int dstx = 0;
	int dsty = 0;

	char titleutf8[1024];
	char classNameutf8[1024];
	WideCharToMultiByte(CP_UTF8, 0, title, -1, titleutf8, 1024 - 1, nullptr, nullptr);
	WideCharToMultiByte(CP_UTF8, 0, windowClassName, -1, classNameutf8, 1024 - 1, nullptr, nullptr);

	HWND hwnd = (HWND)VrInterface::init(inst, titleutf8, classNameutf8);
#else

	if (windowCounter == 0) {
		::registerWindowClass(inst, windowClassName);
	}

	DWORD dwExStyle;
	DWORD dwStyle;

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = width;
	WindowRect.top = 0;
	WindowRect.bottom = height;

	switch (windowMode) {
	default: // fall through
	case WindowModeWindow:
		dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		break;
	case WindowModeFullScreen:
		dwStyle = WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		break;
	case WindowModeExclusiveFullscreen: {
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &startDeviceMode);
		deviceModeChanged = true;

		DEVMODEW dmScreenSettings;                              // Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings)); // Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);     // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;                   // Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;                 // Selected Screen Height
		dmScreenSettings.dmBitsPerPel = 32;                     // Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			// return FALSE;
		}

		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);
		break;
	}
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle); // Adjust Window To True Requested Size

	Kore::Display* displayDevice = targetDisplay < 0 ? Kore::Display::primary() : Kore::Display::get(targetDisplay);

	int dstx = displayDevice->x;
	int dsty = displayDevice->y;
	uint xres = GetSystemMetrics(SM_CXSCREEN);
	uint yres = GetSystemMetrics(SM_CYSCREEN);
	uint w = width;
	uint h = height;

	switch (windowMode) {
	default:               // fall through
	case WindowModeWindow: // fall through
	case WindowModeFullScreen: {
		dstx += x < 0 ? (displayDevice->width - w) >> 1 : x;
		dsty += y < 0 ? (displayDevice->height - h) >> 1 : y;
	} break;

	case WindowModeExclusiveFullscreen: {
		// dstx = 0;
		// dsty = 0;
	} break;
	}

	HWND hwnd = CreateWindowEx(dwExStyle, windowClassName, title, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, dstx, dsty, WindowRect.right - WindowRect.left,
	                           WindowRect.bottom - WindowRect.top, nullptr, nullptr, inst, nullptr);

	if (windowCounter == 0) {
		if (windowMode == WindowModeExclusiveFullscreen) {
			SetWindowPos(hwnd, nullptr, dstx, dsty, width, height, 0);
		}
	}

	GetFocus(); // TODO (DK) that seems like a useless call, as the return value isn't saved anywhere?
	::SetCursor(LoadCursor(0, IDC_ARROW));

	DragAcceptFiles(hwnd, true);

	if (windowCounter == 0) {
		loadXInput();
		initializeDirectInput();
	}
#endif /*#else // #ifdef KORE_OCULUS  */

	/*windows[windowCounter].handle = hwnd;
	windows[windowCounter].x = dstx;
	windows[windowCounter].y = dsty;
	windows[windowCounter].width = width;
	windows[windowCounter].height = height;*/
	return windowCounter;
}

void* Window::handle() {
	WindowsWindow* win = (WindowsWindow*)this;
	return win->handle;
}

void Window::destroy(Window* window) {
	HWND hwnd = (HWND)window->handle();

	// TODO (DK) shouldn't 'hwnd = nullptr' moved out of here?
	if (hwnd && !DestroyWindow(hwnd)) {
		// MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hwnd = nullptr;
	}

	//**window->handle = nullptr;

	// TODO (DK) only unregister after the last window is destroyed?
	if (!UnregisterClass(windowClassName, GetModuleHandle(nullptr))) {
		// MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		// hInstance=NULL;
	}

	--windowCounter;
}

/*
void Kore::System::setTitle(const char* title, int window) {
	wchar_t buffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, buffer, 1024);
	SetWindowText(windows[window]->hwnd, buffer);
}

// TODO (DK) windowId
void Kore::System::showWindow() {
	ShowWindow(windows[0]->hwnd, SW_SHOWDEFAULT);
	UpdateWindow(windows[0]->hwnd);
}
*/

Kore::Window* Kore::Window::create(WindowOptions* win, FramebufferOptions* frame) {
	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, win->title, -1, wbuffer, 1024);

	int windowId = createWindow(wbuffer, win->x, win->y, win->width, win->height, win->mode, win->display);

	HWND hwnd = windows[windowId].handle;
	long style = GetWindowLong(hwnd, GWL_STYLE);

	if (win->windowFeatures & WindowFeatureResizable) {
		style |= WS_SIZEBOX;
	}

	if (win->windowFeatures & WindowFeatureMaximizable) {
		style |= WS_MAXIMIZEBOX;
	}

	if (win->windowFeatures & WindowFeatureMinimizable == 0) {
		style ^= WS_MINIMIZEBOX;
	}

	SetWindowLong(hwnd, GWL_STYLE, style);

	Graphics4::setAntialiasingSamples(frame->samplesPerPixel);
	bool vsync = frame->verticalSync;
#ifdef KORE_OCULUS
	vsync = false;
#endif
	Graphics4::init(windowId, frame->depthBufferBits, frame->stencilBufferBits, vsync);

	return &windows[windowId];
}

/*
void Kore::System::_shutdown() {
	if (windowCounter == 0 && deviceModeChanged) {
		ChangeDisplaySettings(&startDeviceMode, 0);
	}
}
*/
