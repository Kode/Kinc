#pragma once

struct HWND__;

struct WindowData {
	HWND__* handle;
	bool mouseInside;
	int x, y;
	WindowData();
};
