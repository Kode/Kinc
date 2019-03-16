#pragma once

#include <Kore/Window.h>

#include <Kinc/Window.h>

namespace Kore {
	Kore_WindowMode convert(WindowMode mode);
	Kore_WindowOptions convert(WindowOptions *win);
	Kore_FramebufferOptions convert(FramebufferOptions *frame);
}
