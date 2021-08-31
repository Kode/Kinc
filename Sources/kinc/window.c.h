#include "window.h"
#include "display.h"

#include <stdlib.h>

void kinc_window_options_set_defaults(kinc_window_options_t *win) {
	win->title = NULL;
	win->display_index = kinc_primary_display();
	win->mode = KINC_WINDOW_MODE_WINDOW;
	win->x = -1;
	win->y = -1;
	win->width = 800;
	win->height = 600;
	win->visible = true;
	win->window_features = KINC_WINDOW_FEATURE_RESIZEABLE | KINC_WINDOW_FEATURE_MINIMIZABLE | KINC_WINDOW_FEATURE_MAXIMIZABLE;
}

void kinc_framebuffer_options_set_defaults(kinc_framebuffer_options_t *frame) {
	frame->frequency = 60;
	frame->vertical_sync = true;
	frame->color_bits = 32;
	frame->depth_bits = 16;
	frame->stencil_bits = 8;
	frame->samples_per_pixel = 1;
}
