#pragma once

struct HWND__;
typedef unsigned long DWORD;

struct WindowData {
	HWND__* handle;
	bool mouseInside;
	int index;
	int x, y, display, mode, bpp, frequency, features;
	int manualWidth, manualHeight;
	DWORD dwStyle, dwExStyle;
	void (*resizeCallback)(int x, int y);
	WindowData();
};
