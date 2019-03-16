#pragma once

#include <Kore/Window.h>

#include <C/Kore/Window.h>

namespace Kore {
	Kore_WindowMode convert(WindowMode mode);
	Kore_WindowOptions convert(WindowOptions *win);
	Kore_FramebufferOptions convert(FramebufferOptions *frame);
}
