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

		DEVMODEW dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			// return FALSE;
		}

		dwStyle = WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		ShowCursor(FALSE);
		break;
	}
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	Display* display = targetDisplay < 0 ? Display::primary() : Display::get(targetDisplay);

	int dstx = display->x();
	int dsty = display->y();
	int dstw = width;
	int dsth = height;

	switch (windowMode) {
	case WindowModeWindow:
		dstx += x < 0 ? (display->width() - width) / 2 : x;
		dsty += y < 0 ? (display->height() - height) / 2 : y;
		dstw = WindowRect.right - WindowRect.left;
		dsth = WindowRect.bottom - WindowRect.top;
		break;
	case WindowModeFullScreen:
		dstw = display->width();
		dsth = display->height();
		break;
	case WindowModeExclusiveFullscreen:
		// dstx = 0;
		// dsty = 0;
		break;
	}

	HWND hwnd =
	    CreateWindowEx(dwExStyle, windowClassName, title, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, dstx, dsty, dstw, dsth, nullptr, nullptr, inst, nullptr);

	if (windowCounter == 0) {
		if (windowMode == WindowModeExclusiveFullscreen) {
			SetWindowPos(hwnd, nullptr, dstx, dsty, width, height, 0);
		}
	}

	SetCursor(LoadCursor(0, IDC_ARROW));
	DragAcceptFiles(hwnd, true);
#endif

	windows[windowCounter]._data.handle = hwnd;
	windows[windowCounter]._data.x = dstx;
	windows[windowCounter]._data.y = dsty;
	windows[windowCounter]._data.dwStyle = dwStyle;
	windows[windowCounter]._data.dwExStyle = dwExStyle;
	windows[windowCounter]._data.mode = windowMode;
	windows[windowCounter]._data.display = targetDisplay;
	return windowCounter;
}

void Window::resize(int width, int height) {
	if (_data.mode != 0) {
		return;
	}

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRectEx(&rect, _data.dwStyle, FALSE, _data.dwExStyle);
	SetWindowPos(_data.handle, nullptr, x(), y(), rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::move(int x, int y) {
	if (_data.mode != 0) {
		return;
	}

	_data.x = x;
	_data.y = y;
	SetWindowPos(_data.handle, nullptr, x, y, width(), height(), 0);
}

void Window::changeWindowMode(WindowMode mode) {
	switch (mode) {
	case WindowModeWindow:
		break;
	case WindowModeFullScreen: {
		SetWindowLong(_data.handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(_data.handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		Display* display = _data.display < 0 ? Display::primary() : Display::get(_data.display);
		SetWindowPos(_data.handle, nullptr, display->x(), display->y(), display->width(), display->height(), 0);
		break;
	}
	case WindowModeExclusiveFullscreen:
		break;
	}
	_data.mode = mode;
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

void Window::show() {
	ShowWindow(_data.handle, SW_SHOWDEFAULT);
	UpdateWindow(_data.handle);
}

void Window::hide() {
	ShowWindow(_data.handle, SW_HIDE);
	UpdateWindow(_data.handle);
}

void Window::setTitle(const char* title) {
	wchar_t buffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, buffer, 1024);
	SetWindowTextW(_data.handle, buffer);
}

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

WindowData::WindowData() : handle(nullptr), mouseInside(false), resizeCallback(nullptr) {}

Window::Window() {}

void Window::setResizeCallback(void (*value)(int x, int y)) {
	_data.resizeCallback = value;
}
