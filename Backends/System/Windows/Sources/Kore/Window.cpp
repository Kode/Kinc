#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Window.h>
#include <Kore/Windows.h>

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
	int windowCounter = 0;

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

	DWORD getStyle(int features) {
		DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		if (features & WindowFeatureResizable) {
			style |= WS_SIZEBOX;
		}

		if (features & WindowFeatureMaximizable) {
			style |= WS_MAXIMIZEBOX;
		}

		if (features & WindowFeatureMinimizable) {
			style |= WS_MINIMIZEBOX;
		}

		if ((features & WindowFeatureBorderless) == 0) {
			style |= WS_CAPTION | WS_SYSMENU;
		}

		if (features & WindowFeatureOnTop) {
			style |= WS_POPUP;
		}

		return style;
	}

	DWORD getExStyle(int features) {
		DWORD exStyle = WS_EX_APPWINDOW;

		if ((features & WindowFeatureBorderless) == 0) {
			exStyle |= WS_EX_WINDOWEDGE;
		}

		return exStyle;
	}
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

bool setDisplayMode(Display* display, int width, int height, int bpp, int frequency);

static int createWindow(const wchar_t* title, int x, int y, int width, int height, int bpp, int frequency, int features, Kore::WindowMode windowMode,
                        Display* targetDisplay) {
	HINSTANCE inst = GetModuleHandle(nullptr);
#ifdef KORE_OCULUS
	if (windowCounter == 0) {
		::registerWindowClass(inst, windowClassName);
	}
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

	Display* display = targetDisplay == nullptr ? Display::primary() : targetDisplay;

	DWORD dwStyle, dwExStyle;

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = width;
	WindowRect.top = 0;
	WindowRect.bottom = height;

	switch (windowMode) {
	case WindowModeWindow:
		dwStyle = getStyle(features);
		dwExStyle = getExStyle(features);
		break;
	case WindowModeFullscreen:
		dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		break;
	case WindowModeExclusiveFullscreen: {
		setDisplayMode(display, width, height, bpp, frequency);
		dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		break;
	}
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

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
	case WindowModeFullscreen:
		dstw = display->width();
		dsth = display->height();
		break;
	case WindowModeExclusiveFullscreen:
		break;
	}

	HWND hwnd =
	    CreateWindowEx(dwExStyle, windowClassName, title, dwStyle, dstx, dsty, dstw, dsth, nullptr, nullptr, inst, nullptr);

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
	windows[windowCounter]._data.bpp = bpp;
	windows[windowCounter]._data.frequency = frequency;
	windows[windowCounter]._data.features = features;
	windows[windowCounter]._data.manualWidth = width;
	windows[windowCounter]._data.manualHeight = height;
	windows[windowCounter]._data.index = windowCounter;

	return windowCounter++;
}

void Window::resize(int width, int height) {
	_data.manualWidth = width;
	_data.manualHeight = height;
	switch (_data.mode) { 
	case WindowModeWindow: {
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRectEx(&rect, _data.dwStyle, FALSE, _data.dwExStyle);
		SetWindowPos(_data.handle, nullptr, x(), y(), rect.right - rect.left, rect.bottom - rect.top, 0);
		break;
	}
	case WindowModeExclusiveFullscreen: {
		Display* display = this->display();
		setDisplayMode(display, width, height, _data.bpp, _data.frequency);
		SetWindowPos(_data.handle, nullptr, display->x(), display->y(), display->width(), display->height(), 0);
		break;
	}
	}
}

void Window::move(int x, int y) {
	if (_data.mode != 0) {
		return;
	}

	_data.x = x;
	_data.y = y;
	SetWindowPos(_data.handle, nullptr, x, y, width(), height(), 0);
}

void Window::changeFramebuffer(FramebufferOptions* frame) {
	Graphics4::_changeFramebuffer(_data.index, frame);
}

void Window::changeWindowFeatures(int features) {
	_data.features = features;
	SetWindowLong(_data.handle, GWL_STYLE, getStyle(features));
	SetWindowLong(_data.handle, GWL_EXSTYLE, getExStyle(features));
}

void Window::changeWindowMode(WindowMode mode) {
	Display* display = this->display();
	switch (mode) {
	case WindowModeWindow:
		Windows::restoreDisplay(display->_data.index);
		changeWindowFeatures(_data.features);
		break;
	case WindowModeFullscreen: {
		Windows::restoreDisplay(display->_data.index);
		SetWindowLong(_data.handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(_data.handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(_data.handle, nullptr, display->x(), display->y(), display->width(), display->height(), 0);
		break;
	}
	case WindowModeExclusiveFullscreen:
		setDisplayMode(display, _data.manualWidth, _data.manualHeight, _data.bpp, _data.frequency);
		SetWindowLong(_data.handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(_data.handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(_data.handle, nullptr, display->x(), display->y(), display->width(), display->height(), 0);
		break;
	}
	_data.mode = mode;
}

WindowMode Window::mode() {
	return (WindowMode)_data.mode;
}

void Window::destroy(Window* window) {
	if (window->_data.handle != nullptr) {
		DestroyWindow(window->_data.handle);
		window->_data.handle = nullptr;
		--windowCounter;
	}
}

void Windows::hideWindows() {
	for (int i = 0; i < maximumWindows; ++i) {
		if (windows[i]._data.handle != nullptr) {
			ShowWindow(windows[i]._data.handle, SW_HIDE);
			UpdateWindow(windows[i]._data.handle);
		}
	}
}

void Windows::destroyWindows() {
	for (int i = 0; i < maximumWindows; ++i) {
		Window::destroy(&windows[i]);
	}
	UnregisterClass(windowClassName, GetModuleHandle(nullptr));
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

	int windowId =
	    createWindow(wbuffer, win->x, win->y, win->width, win->height, frame->colorBufferBits, frame->frequency, win->windowFeatures, win->mode, win->display);

	Graphics4::setAntialiasingSamples(frame->samplesPerPixel);
	bool vsync = frame->verticalSync;
#ifdef KORE_OCULUS
	vsync = false;
#endif
	Graphics4::init(windowId, frame->depthBufferBits, frame->stencilBufferBits, vsync);

	if (win->visible) {
		windows[windowId].show();
	}

	return &windows[windowId];
}

WindowData::WindowData() : handle(nullptr), mouseInside(false), resizeCallback(nullptr) {}

Window::Window() {}

void Window::setResizeCallback(void (*value)(int x, int y)) {
	_data.resizeCallback = value;
}

Display* Window::display() {
	return Windows::getDisplayForMonitor(MonitorFromWindow(_data.handle, MONITOR_DEFAULTTOPRIMARY));
}
