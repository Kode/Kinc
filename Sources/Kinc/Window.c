#include "pch.h"

#include "Window.h"

void Kinc_Internal_InitWindowOptions(Kinc_WindowOptions *win) {
	win->title = "Kore";
	win->display_index = 0;
	win->mode = KINC_WINDOW_MODE_WINDOW;
	win->x = -1;
	win->y = -1;
	win->width = 800;
	win->height = 600;
	win->visible = true;
	win->window_features = KINC_WINDOW_FEATURE_RESIZEABLE | KINC_WINDOW_FEATURE_MINIMIZABLE | KINC_WINDOW_FEATURE_MAXIMIZABLE;
}

void Kinc_Internal_InitFramebufferOptions(Kinc_FramebufferOptions *frame) {
	frame->frequency = 60;
	frame->vertical_sync = true;
	frame->color_bits = 32;
	frame->depth_bits = 16;
	frame->stencil_bits = 8;
	frame->samples_per_pixel = 1;
}
