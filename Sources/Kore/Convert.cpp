#include "pch.h"

#include "Convert.h"

#include <Kore/Display.h>

using namespace Kore;

Kore_WindowMode Kore::convert(WindowMode mode) {
	switch (mode) {
	case WindowModeWindow:
		return WINDOW_MODE_WINDOW;
	case WindowModeFullscreen:
		return WINDOW_MODE_FULLSCREEN;
	case WindowModeExclusiveFullscreen:
		return WINDOW_MODE_EXCLUSIVE_FULLSCREEN;
	}
	return WINDOW_MODE_WINDOW;
}

Kore_WindowOptions Kore::convert(WindowOptions *win) {
	Kore_WindowOptions kwin;
	kwin.title = win->title;
	kwin.x = win->x;
	kwin.y = win->y;
	kwin.width = win->width;
	kwin.height = win->height;
	kwin.display_index = win->display == nullptr ? -1 : win->display->_index;
	kwin.visible = win->visible;
	kwin.window_features = win->windowFeatures;
	kwin.mode = convert(win->mode);
	return kwin;
}

Kore_FramebufferOptions Kore::convert(FramebufferOptions *frame) {
	Kore_FramebufferOptions kframe;
	kframe.frequency = frame->frequency;
	kframe.vertical_sync = frame->verticalSync;
	kframe.color_bits = frame->colorBufferBits;
	kframe.depth_bits = frame->depthBufferBits;
	kframe.stencil_bits = frame->stencilBufferBits;
	kframe.samples_per_pixel = frame->samplesPerPixel;
	return kframe;
}
