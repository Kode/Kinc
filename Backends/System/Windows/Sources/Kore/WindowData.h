#pragma once

struct HWND__;
typedef unsigned long DWORD;

struct WindowData {
	HWND__* handle;
	bool mouseInside;
	int x, y;
	DWORD dwStyle, dwExStyle;
	WindowData();
};
