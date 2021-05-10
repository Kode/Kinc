#include "Convert.h"

#include <Kore/Display.h>

using namespace Kore;

kinc_window_mode_t Kore::convert(WindowMode mode) {
	switch (mode) {
	case WindowModeWindow:
		return KINC_WINDOW_MODE_WINDOW;
	case WindowModeFullscreen:
		return KINC_WINDOW_MODE_FULLSCREEN;
	case WindowModeExclusiveFullscreen:
		return KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN;
	}
	return KINC_WINDOW_MODE_WINDOW;
}

kinc_window_options_t Kore::convert(WindowOptions *win) {
	kinc_window_options_t kwin;
	kwin.title = win->title;
	kwin.x = win->x;
	kwin.y = win->y;
	kwin.width = win->width;
	kwin.height = win->height;
	kwin.display_index = win->displayIndex;
	kwin.visible = win->visible;
	kwin.window_features = win->windowFeatures;
	kwin.mode = convert(win->mode);
	return kwin;
}

kinc_framebuffer_options_t Kore::convert(FramebufferOptions *frame) {
	kinc_framebuffer_options_t kframe;
	kframe.frequency = frame->frequency;
	kframe.vertical_sync = frame->verticalSync;
	kframe.color_bits = frame->colorBufferBits;
	kframe.depth_bits = frame->depthBufferBits;
	kframe.stencil_bits = frame->stencilBufferBits;
	kframe.samples_per_pixel = frame->samplesPerPixel;
	return kframe;
}
