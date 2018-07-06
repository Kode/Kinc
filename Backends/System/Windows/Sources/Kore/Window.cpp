#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Window.h>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>

#undef CreateWindow

using namespace Kore;

LRESULT WINAPI KoreWindowsMessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	const int maximumWindows = 10;
	Window windows[maximumWindows];
	int windowCounter = -1;

#ifdef KORE_OCULUS
	const wchar_t* windowClassName = L"ORT";
#else
	const wchar_t* windowClassName = L"KoreWindow";
#endif

	void registerWindowClass(HINSTANCE hInstance, const wchar_t* className) {
		WNDCLASSEXW wc = {sizeof(WNDCLASSEXA),
		                  CS_OWNDC /*CS_CLASSDC*/,
		                  KoreWindowsMessageProcedure,
		                  0L,
		                  0L,
		                  hInstance,
		                  LoadIcon(hInstance, MAKEINTRESOURCE(107)),
		                  LoadCursor(nullptr, IDC_ARROW),
		                  0,
		                  0,
		                  className,
		                  0};
		RegisterClassEx(&wc);
	}

	bool deviceModeChanged = false;
	DEVMODE startDeviceMode;
}

int idFromHWND(HWND handle) {
	for (int i = 0; i < maximumWindows; ++i) {
		if (windows[i]._data.handle == handle) {
			return i;
		}
	}
	return -1;
}

Window* Window::get(int window) {
	return &windows[window];
}

int Window::count() {
	return windowCounter;
}

int Window::x() {
	return _data.x;
}

int Window::y() {
	return _data.y;
}

int Window::width() {
	RECT rect;
	GetClientRect(_data.handle, &rect);
	return rect.right;
}

int Window::height() {
	RECT rect;
	GetClientRect(_data.handle, &rect);
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

	DWORD dwStyle, dwExStyle;

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

	int dstx = displayDevice->x();
	int dsty = displayDevice->y();
	uint xres = GetSystemMetrics(SM_CXSCREEN);
	uint yres = GetSystemMetrics(SM_CYSCREEN);
	uint w = width;
	uint h = height;

	switch (windowMode) {
	default:               // fall through
	case WindowModeWindow: // fall through
	case WindowModeFullScreen: {
		dstx += x < 0 ? (displayDevice->width() - w) >> 1 : x;
		dsty += y < 0 ? (displayDevice->height() - h) >> 1 : y;
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
#endif

	windows[windowCounter]._data.handle = hwnd;
	windows[windowCounter]._data.x = dstx;
	windows[windowCounter]._data.y = dsty;
	windows[windowCounter]._data.dwStyle = dwStyle;
	windows[windowCounter]._data.dwExStyle = dwExStyle;
	return windowCounter;
}

void Window::resize(int width, int height) {
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRectEx(&rect, _data.dwStyle, FALSE, _data.dwExStyle);
	SetWindowPos(_data.handle, nullptr, x(), y(), rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::move(int x, int y) {
	_data.x = x;
	_data.y = y;
	SetWindowPos(_data.handle, nullptr, x, y, width(), height(), 0);
}

void Window::changeWindowMode(WindowMode mode) {

}

void Window::destroy(Window* window) {
	if (window->_data.handle != nullptr) {
		DestroyWindow(window->_data.handle);
	}
	window->_data.handle = nullptr;

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
	WindowOptions defaultWin;
	FramebufferOptions defaultFrame;

	if (win == nullptr) {
		win = &defaultWin;
	}

	if (frame == nullptr) {
		frame = &defaultFrame;
	}

	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, win->title, -1, wbuffer, 1024);

	int windowId = createWindow(wbuffer, win->x, win->y, win->width, win->height, win->mode, win->display);

	long style = GetWindowLong(windows[windowId]._data.handle, GWL_STYLE);

	if (win->windowFeatures & WindowFeatureResizable) {
		style |= WS_SIZEBOX;
	}

	if (win->windowFeatures & WindowFeatureMaximizable) {
		style |= WS_MAXIMIZEBOX;
	}

	if ((win->windowFeatures & WindowFeatureMinimizable) == 0) {
		style ^= WS_MINIMIZEBOX;
	}

	SetWindowLong(windows[windowId]._data.handle, GWL_STYLE, style);

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

WindowData::WindowData() : handle(nullptr), mouseInside(false) {}

Window::Window() {}
