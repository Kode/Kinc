#include "pch.h"

#include <Kinc/Window.h>
#include <Kinc/Display.h>
#include <Kinc/Bridge.h>

#include <Kore/Windows.h>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>

#undef CreateWindow

struct HWND__;
typedef unsigned long DWORD;

typedef struct {
	struct HWND__ *handle;
	int display_index;
	bool mouseInside;
	int index;
	int x, y, mode, bpp, frequency, features;
	int manualWidth, manualHeight;
	DWORD dwStyle, dwExStyle;
	void (*resizeCallback)(int x, int y, void* data);
	void* resizeCallbackData;
	void (*ppiCallback)(int ppi, void* data);
	void* ppiCallbackData;
} WindowData;

LRESULT WINAPI KoreWindowsMessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define MAXIMUM_WINDOWS 16
static WindowData windows[MAXIMUM_WINDOWS] = {0};
static int window_counter = 0;

#ifdef KORE_OCULUS
	const wchar_t *windowClassName = L"ORT";
#else
	const wchar_t *windowClassName = L"KoreWindow";
#endif

static void RegisterWindowClass(HINSTANCE hInstance, const wchar_t *className) {
	WNDCLASSEXW wc = {sizeof(WNDCLASSEXA),
		                CS_OWNDC /*CS_CLASSDC*/,
		                KoreWindowsMessageProcedure,
		                0L,
		                0L,
		                hInstance,
		                LoadIcon(hInstance, MAKEINTRESOURCE(107)),
		                LoadCursor(NULL, IDC_ARROW),
		                0,
		                0,
		                className,
		                0};
	RegisterClassEx(&wc);
}

static DWORD getStyle(int features) {
	DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (features & KINC_WINDOW_FEATURE_RESIZEABLE) {
		style |= WS_SIZEBOX;
	}

	if (features & KINC_WINDOW_FEATURE_MAXIMIZABLE) {
		style |= WS_MAXIMIZEBOX;
	}

	if (features & KINC_WINDOW_FEATURE_MINIMIZABLE) {
		style |= WS_MINIMIZEBOX;
	}

	if ((features & KINC_WINDOW_FEATURE_BORDERLESS) == 0) {
		style |= WS_CAPTION | WS_SYSMENU;
	}

	if (features & KINC_WINDOW_FEATURE_ON_TOP) {
		style |= WS_POPUP;
	}

	return style;
}

static DWORD getExStyle(int features) {
	DWORD exStyle = WS_EX_APPWINDOW;

	if ((features & KINC_WINDOW_FEATURE_BORDERLESS) == 0) {
		exStyle |= WS_EX_WINDOWEDGE;
	}

	return exStyle;
}

int Kinc_Windows_WindowIndexFromHWND(struct HWND__ *handle) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		if (windows[i].handle == handle) {
			return i;
		}
	}
	return -1;
}

int Kinc_CountWindows() {
	return window_counter;
}

int Kinc_WindowX(int window_index) {
	return windows[window_index].x;
}

int Kinc_WindowY(int window_index) {
	return windows[window_index].y;
}

int Kinc_WindowWidth(int window_index) {
	RECT rect;
	GetClientRect(windows[window_index].handle, &rect);
	return rect.right;
}

int Kinc_WindowHeight(int window_index) {
	RECT rect;
	GetClientRect(windows[window_index].handle, &rect);
	return rect.bottom;
}

static int createWindow(const wchar_t *title, int x, int y, int width, int height, int bpp, int frequency, int features, Kinc_WindowMode windowMode,
                        int target_display_index) {
	HINSTANCE inst = GetModuleHandle(NULL);
#ifdef KORE_OCULUS
	if (windowCounter == 0) {
		RegisterWindowClass(inst, windowClassName);
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

	if (window_counter == 0) {
		RegisterWindowClass(inst, windowClassName);
	}

	int display_index = target_display_index == -1 ? Kinc_PrimaryDisplay() : target_display_index;

	DWORD dwStyle, dwExStyle;

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = width;
	WindowRect.top = 0;
	WindowRect.bottom = height;

	switch (windowMode) {
	case KINC_WINDOW_MODE_WINDOW:
		dwStyle = getStyle(features);
		dwExStyle = getExStyle(features);
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
		dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		break;
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN: {
		Kinc_Windows_SetDisplayMode(display_index, width, height, bpp, frequency);
		dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		break;
	}
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	Kinc_DisplayMode display_mode = Kinc_DisplayCurrentMode(display_index);

	int dstx = display_mode.x;
	int dsty = display_mode.y;
	int dstw = width;
	int dsth = height;

	switch (windowMode) {
	case KINC_WINDOW_MODE_WINDOW:
		dstx += x < 0 ? (display_mode.width - width) / 2 : x;
		dsty += y < 0 ? (display_mode.height - height) / 2 : y;
		dstw = WindowRect.right - WindowRect.left;
		dsth = WindowRect.bottom - WindowRect.top;
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
		dstw = display_mode.width;
		dsth = display_mode.height;
		break;
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
		break;
	}

	HWND hwnd = CreateWindowEx(dwExStyle, windowClassName, title, dwStyle, dstx, dsty, dstw, dsth, NULL, NULL, inst, NULL);

	SetCursor(LoadCursor(0, IDC_ARROW));
	DragAcceptFiles(hwnd, true);
#endif

	windows[window_counter].handle = hwnd;
	windows[window_counter].x = dstx;
	windows[window_counter].y = dsty;
	windows[window_counter].dwStyle = dwStyle;
	windows[window_counter].dwExStyle = dwExStyle;
	windows[window_counter].mode = windowMode;
	windows[window_counter].display_index = display_index;
	windows[window_counter].bpp = bpp;
	windows[window_counter].frequency = frequency;
	windows[window_counter].features = features;
	windows[window_counter].manualWidth = width;
	windows[window_counter].manualHeight = height;
	windows[window_counter].index = window_counter;

	return window_counter++;
}

void Kinc_WindowResize(int window_index, int width, int height) {
	WindowData *win = &windows[window_index];
	win->manualWidth = width;
	win->manualHeight = height;
	switch (win->mode) {
	case KINC_WINDOW_MODE_WINDOW: {
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRectEx(&rect, win->dwStyle, FALSE, win->dwExStyle);
		SetWindowPos(win->handle, NULL, Kinc_WindowX(window_index), Kinc_WindowY(window_index), rect.right - rect.left, rect.bottom - rect.top, 0);
		break;
	}
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN: {
		int display_index = Kinc_WindowDisplay(window_index);
		Kinc_DisplayMode display_mode = Kinc_DisplayCurrentMode(display_index);
		Kinc_Windows_SetDisplayMode(display_index, width, height, win->bpp, win->frequency);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		break;
	}
	}
}

void Kinc_WindowMove(int window_index, int x, int y) {
	WindowData *win = &windows[window_index];

	if (win->mode != 0) {
		return;
	}

	win->x = x;
	win->y = y;
	SetWindowPos(win->handle, NULL, x, y, Kinc_WindowWidth(window_index), Kinc_WindowHeight(window_index), 0);
}

void Kinc_WindowChangeFramebuffer(int window_index, Kinc_FramebufferOptions *frame) {
	Kinc_Bridge_G4_Internal_ChangeFramebuffer(window_index, frame);
}

void Kinc_WindowChangeFeatures(int window_index, int features) {
	WindowData *win = &windows[window_index];
	win->features = features;
	SetWindowLong(win->handle, GWL_STYLE, getStyle(features));
	SetWindowLong(win->handle, GWL_EXSTYLE, getExStyle(features));
}

void Kinc_WindowChangeMode(int window_index, Kinc_WindowMode mode) {
	WindowData *win = &windows[window_index];
	int display_index = Kinc_WindowDisplay(window_index);
	Kinc_DisplayMode display_mode = Kinc_DisplayCurrentMode(display_index);
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW:
		Kinc_Windows_RestoreDisplay(display_index);
		Kinc_WindowChangeFeatures(window_index, win->features);
		break;
	case KINC_WINDOW_MODE_FULLSCREEN: {
		Kinc_Windows_RestoreDisplay(display_index);
		SetWindowLong(win->handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(win->handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		break;
	}
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
		Kinc_Windows_SetDisplayMode(display_index, win->manualWidth, win->manualHeight, win->bpp, win->frequency);
		SetWindowLong(win->handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(win->handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		break;
	}
	win->mode = mode;
}

Kinc_WindowMode Kinc_WindowGetMode(int window_index) {
	return (Kinc_WindowMode)windows[window_index].mode;
}

void Kinc_WindowDestroy(int window_index) {
	WindowData *win = &windows[window_index];
	if (win->handle != NULL) {
		DestroyWindow(win->handle);
		win->handle = NULL;
		--window_counter;
	}
}

void Kinc_Windows_HideWindows(void) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		if (windows[i].handle != NULL) {
			ShowWindow(windows[i].handle, SW_HIDE);
			UpdateWindow(windows[i].handle);
		}
	}
}

void Kinc_Windows_DestroyWindows(void) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		Kinc_WindowDestroy(i);
	}
	UnregisterClass(windowClassName, GetModuleHandle(NULL));
}

void Kinc_WindowShow(int window_index) {
	ShowWindow(windows[window_index].handle, SW_SHOWDEFAULT);
	UpdateWindow(windows[window_index].handle);
}

void Kinc_WindowHide(int window_index) {
	ShowWindow(windows[window_index].handle, SW_HIDE);
	UpdateWindow(windows[window_index].handle);
}

void Kinc_WindowSetTitle(int window_index, const char *title) {
	wchar_t buffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, buffer, 1024);
	SetWindowTextW(windows[window_index].handle, buffer);
}

int Kinc_WindowCreate(Kinc_WindowOptions *win, Kinc_FramebufferOptions *frame) {
	Kinc_WindowOptions defaultWin;
	Kinc_FramebufferOptions defaultFrame;

	if (win == NULL) {
		Kinc_Internal_InitWindowOptions(&defaultWin);
		win = &defaultWin;
	}

	if (frame == NULL) {
		Kinc_Internal_InitFramebufferOptions(&defaultFrame);
		frame = &defaultFrame;
	}

	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, win->title, -1, wbuffer, 1024);

	int windowId =
	    createWindow(wbuffer, win->x, win->y, win->width, win->height, frame->color_bits, frame->frequency, win->window_features, win->mode, win->display_index);

	Kinc_Bridge_G4_Internal_SetAntialiasingSamples(frame->samples_per_pixel);
	bool vsync = frame->vertical_sync;
#ifdef KORE_OCULUS
	vsync = false;
#endif
	Kinc_Bridge_G4_Internal_Init(windowId, frame->depth_bits, frame->stencil_bits, vsync);

	if (win->visible) {
		Kinc_WindowShow(windowId);
	}

	return windowId;
}

void Kinc_WindowSetResizeCallback(int window_index, void (*callback)(int x, int y, void *data), void *data) {
	windows[window_index].resizeCallback = callback;
	windows[window_index].resizeCallbackData = data;
}

void Kinc_WindowSetPpiChangedCallback(int window_index, void (*callback)(int ppi, void *data), void *data) {
	windows[window_index].ppiCallback = callback;
	windows[window_index].ppiCallbackData = data;
}

int Kinc_WindowDisplay(int window_index) {
	return Kinc_Windows_GetDisplayForMonitor(MonitorFromWindow(windows[window_index].handle, MONITOR_DEFAULTTOPRIMARY));
}

struct HWND__ *Kinc_Windows_WindowHandle(int window_index) {
	return windows[window_index].handle;
}

int Kinc_Windows_ManualWidth(int window) {
	return windows[window].manualWidth;
}

int Kinc_Windows_ManualHeight(int window) {
	return windows[window].manualHeight;
}

void Kinc_Internal_CallResizeCallback(int window_index, int width, int height) {
	if (windows[window_index].resizeCallback != NULL) {
		windows[window_index].resizeCallback(width, height, windows[window_index].resizeCallbackData);
	}
}

void Kinc_Internal_CallPpiChangedCallback(int window_index, int ppi) {
	if (windows[window_index].ppiCallback != NULL) {
		windows[window_index].ppiCallback(ppi, windows[window_index].ppiCallbackData);
	}
}
