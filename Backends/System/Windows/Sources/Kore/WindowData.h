#pragma once

struct HWND__;
typedef unsigned long DWORD;

namespace Kore {
	class Display;

	struct WindowData {
		HWND__* handle;
		Display* display;
		bool mouseInside;
		int index;
		int x, y, mode, bpp, frequency, features;
		int manualWidth, manualHeight;
		DWORD dwStyle, dwExStyle;
		void (*resizeCallback)(int x, int y);
		WindowData();
	};
}
