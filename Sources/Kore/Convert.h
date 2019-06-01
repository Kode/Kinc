#pragma once

#include <Kore/Window.h>

#include <kinc/window.h>

namespace Kore {
	kinc_window_mode_t convert(WindowMode mode);
	kinc_window_options_t convert(WindowOptions *win);
	kinc_framebuffer_options_t convert(FramebufferOptions *frame);
}
