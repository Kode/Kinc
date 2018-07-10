#pragma once

#include <objc/runtime.h>

namespace Kore {
	struct WindowData {
		id handle;
		id view;
		void (*resizeCallback)(int width, int height);
		WindowData();
	};
}
