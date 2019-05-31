#pragma once

#include <Kore/Window.h>

#include <kinc/window.h>

namespace Kore {
	Kinc_WindowMode convert(WindowMode mode);
	Kinc_WindowOptions convert(WindowOptions *win);
	Kinc_FramebufferOptions convert(FramebufferOptions *frame);
}
