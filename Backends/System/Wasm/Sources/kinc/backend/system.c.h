#include <kinc/audio2/audio.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <stdio.h>
#include <stdlib.h>

int kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}
	win->width = width;
	win->height = height;

	kinc_g4_internal_init();
	kinc_g4_internal_init_window(0, frame->depth_bits, frame->stencil_bits, true);
	return 0;
}

bool kinc_internal_handle_messages() {
	return true;
}

void kinc_set_keep_screen_on(bool on) {}

double kinc_frequency(void) {
	return 1000.0;
}

kinc_ticks_t kinc_timestamp(void) {
	return (kinc_ticks_t)(0.0);
}

double kinc_time(void) {
	return 0.0;
}

extern int kickstart(int argc, char **argv);
