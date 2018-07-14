#pragma once

#include <objc/runtime.h>

namespace Kore {
	struct WindowData {
		id handle;
		id view;
		bool fullscreen;
		void (*resizeCallback)(int width, int height, void* data);
		void* resizeCallbackData;
		WindowData();
	};
}
