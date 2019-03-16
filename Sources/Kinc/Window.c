#include "pch.h"

#include "Window.h"

void Kore_Internal_InitWindowOptions(Kore_WindowOptions *win) {
	win->title = "Kore";
	win->display_index = 0;
	win->mode = WINDOW_MODE_WINDOW;
	win->x = -1;
	win->y = -1;
	win->width = 800;
	win->height = 600;
	win->visible = true;
	win->window_features = KORE_WINDOW_FEATURE_RESIZEABLE | KORE_WINDOW_FEATURE_MINIMIZABLE | KORE_WINDOW_FEATURE_MAXIMIZABLE;
}

void Kore_Internal_InitFramebufferOptions(Kore_FramebufferOptions *frame) {
	frame->frequency = 60;
	frame->vertical_sync = true;
	frame->color_bits = 32;
	frame->depth_bits = 16;
	frame->stencil_bits = 8;
	frame->samples_per_pixel = 1;
}
