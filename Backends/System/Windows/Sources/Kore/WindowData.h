#pragma once

struct HWND__;
typedef unsigned long DWORD;

struct WindowData {
	HWND__* handle;
	bool mouseInside;
	int x, y, display, mode;
	DWORD dwStyle, dwExStyle;
	void (*resizeCallback)(int x, int y);
	WindowData();
};
