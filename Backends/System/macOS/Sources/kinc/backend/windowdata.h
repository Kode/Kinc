#pragma once

#include <objc/runtime.h>

struct WindowData {
	id handle;
	id view;
	bool fullscreen;
	void (*resizeCallback)(int width, int height, void *data);
	void *resizeCallbackData;
};

NSWindow *kinc_get_mac_window_handle(int window_index);
