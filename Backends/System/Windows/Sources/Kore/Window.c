#include "pch.h"

#include <kinc/window.h>
#include <kinc/display.h>
#include <kinc/bridge.h>

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

int kinc_windows_window_index_from_hwnd(struct HWND__ *handle) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		if (windows[i].handle == handle) {
			return i;
		}
	}
	return -1;
}

int kinc_count_windows() {
	return window_counter;
}

int kinc_window_x(int window_index) {
	RECT rect;
	GetWindowRect(windows[window_index].handle, &rect);
	windows[window_index].x = rect.left;
	return windows[window_index].x;
}

int kinc_window_y(int window_index) {
	RECT rect;
	GetWindowRect(windows[window_index].handle, &rect);
	windows[window_index].y = rect.top;
	return windows[window_index].y;
}

int kinc_window_width(int window_index) {
	RECT rect;
	GetClientRect(windows[window_index].handle, &rect);
	return rect.right;
}

int kinc_window_height(int window_index) {
	RECT rect;
	GetClientRect(windows[window_index].handle, &rect);
	return rect.bottom;
}

static int createWindow(const wchar_t *title, int x, int y, int width, int height, int bpp, int frequency, int features, kinc_window_mode_t windowMode,
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

	int display_index = target_display_index == -1 ? kinc_primary_display() : target_display_index;

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
		kinc_windows_set_display_mode(display_index, width, height, bpp, frequency);
		dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW;
		break;
	}
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	kinc_display_mode_t display_mode = kinc_display_current_mode(display_index);

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

void kinc_window_resize(int window_index, int width, int height) {
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
		SetWindowPos(win->handle, NULL, kinc_window_x(window_index), kinc_window_y(window_index), rect.right - rect.left, rect.bottom - rect.top, 0);
		break;
	}
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN: {
		int display_index = kinc_window_display(window_index);
		kinc_display_mode_t display_mode = kinc_display_current_mode(display_index);
		kinc_windows_set_display_mode(display_index, width, height, win->bpp, win->frequency);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		break;
	}
	}
}

void kinc_window_move(int window_index, int x, int y) {
	WindowData *win = &windows[window_index];

	if (win->mode != 0) {
		return;
	}

	win->x = x;
	win->y = y;
	SetWindowPos(win->handle, NULL, x, y, kinc_window_width(window_index), kinc_window_height(window_index), 0);
}

void kinc_window_change_framebuffer(int window_index, kinc_framebuffer_options_t *frame) {
	kinc_bridge_g4_internal_change_framebuffer(window_index, frame);
}

void kinc_window_change_features(int window_index, int features) {
	WindowData *win = &windows[window_index];
	win->features = features;
	SetWindowLong(win->handle, GWL_STYLE, getStyle(features));
	SetWindowLong(win->handle, GWL_EXSTYLE, getExStyle(features));
}

void kinc_window_change_mode(int window_index, kinc_window_mode_t mode) {
	WindowData *win = &windows[window_index];
	int display_index = kinc_window_display(window_index);
	kinc_display_mode_t display_mode = kinc_display_current_mode(display_index);
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW:
		kinc_windows_restore_display(display_index);
		kinc_window_change_features(window_index, win->features);
		break;
	case KINC_WINDOW_MODE_FULLSCREEN: {
		kinc_windows_restore_display(display_index);
		SetWindowLong(win->handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(win->handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		break;
	}
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
		kinc_windows_set_display_mode(display_index, win->manualWidth, win->manualHeight, win->bpp, win->frequency);
		SetWindowLong(win->handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLong(win->handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		break;
	}
	win->mode = mode;
}

kinc_window_mode_t kinc_window_get_mode(int window_index) {
	return (kinc_window_mode_t)windows[window_index].mode;
}

void kinc_window_destroy(int window_index) {
	WindowData *win = &windows[window_index];
	if (win->handle != NULL) {
		DestroyWindow(win->handle);
		win->handle = NULL;
		--window_counter;
	}
}

void kinc_windows_hide_windows(void) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		if (windows[i].handle != NULL) {
			ShowWindow(windows[i].handle, SW_HIDE);
			UpdateWindow(windows[i].handle);
		}
	}
}

void kinc_windows_destroy_windows(void) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		kinc_window_destroy(i);
	}
	UnregisterClass(windowClassName, GetModuleHandle(NULL));
}

void kinc_window_show(int window_index) {
	ShowWindow(windows[window_index].handle, SW_SHOWDEFAULT);
	UpdateWindow(windows[window_index].handle);
}

void kinc_window_hide(int window_index) {
	ShowWindow(windows[window_index].handle, SW_HIDE);
	UpdateWindow(windows[window_index].handle);
}

void kinc_window_set_title(int window_index, const char *title) {
	wchar_t buffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, buffer, 1024);
	SetWindowTextW(windows[window_index].handle, buffer);
}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_window_options_t defaultWin;
	kinc_framebuffer_options_t defaultFrame;

	if (win == NULL) {
		kinc_internal_init_window_options(&defaultWin);
		win = &defaultWin;
	}

	if (frame == NULL) {
		kinc_internal_init_framebuffer_options(&defaultFrame);
		frame = &defaultFrame;
	}

	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, win->title, -1, wbuffer, 1024);

	int windowId =
	    createWindow(wbuffer, win->x, win->y, win->width, win->height, frame->color_bits, frame->frequency, win->window_features, win->mode, win->display_index);

	kinc_bridge_g4_internal_set_antialiasing_samples(frame->samples_per_pixel);
	bool vsync = frame->vertical_sync;
#ifdef KORE_OCULUS
	vsync = false;
#endif
	kinc_bridge_g4_internal_init(windowId, frame->depth_bits, frame->stencil_bits, vsync);

	if (win->visible) {
		kinc_window_show(windowId);
	}

	return windowId;
}

void kinc_window_set_resize_callback(int window_index, void (*callback)(int x, int y, void *data), void *data) {
	windows[window_index].resizeCallback = callback;
	windows[window_index].resizeCallbackData = data;
}

void kinc_window_set_ppi_changed_callback(int window_index, void (*callback)(int ppi, void *data), void *data) {
	windows[window_index].ppiCallback = callback;
	windows[window_index].ppiCallbackData = data;
}

int kinc_window_display(int window_index) {
	return kinc_windows_get_display_for_monitor(MonitorFromWindow(windows[window_index].handle, MONITOR_DEFAULTTOPRIMARY));
}

struct HWND__ *kinc_windows_window_handle(int window_index) {
	return windows[window_index].handle;
}

int kinc_windows_manual_width(int window) {
	return windows[window].manualWidth;
}

int kinc_windows_manual_height(int window) {
	return windows[window].manualHeight;
}

void kinc_internal_call_resize_callback(int window_index, int width, int height) {
	if (windows[window_index].resizeCallback != NULL) {
		windows[window_index].resizeCallback(width, height, windows[window_index].resizeCallbackData);
	}
}

void kinc_internal_call_ppi_changed_callback(int window_index, int ppi) {
	if (windows[window_index].ppiCallback != NULL) {
		windows[window_index].ppiCallback(ppi, windows[window_index].ppiCallbackData);
	}
}
