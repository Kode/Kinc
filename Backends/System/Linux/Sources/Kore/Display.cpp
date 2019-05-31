#include "pch.h"

#include <kinc/display.h>

#include <X11/Xatom.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include <stdlib.h>

int kinc_count_displays(void) {
	return 1;
}

int kinc_primary_display(void) {
	return 0;
}

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
    kinc_display_mode_t m;
    m.x = 0;
    m.y = 0;
	m.width = 800;
	m.height = 600;
	m.frequency = 60;
	m.bits_per_pixel = 32;
	m.pixels_per_inch = 96;
	return m;
}

int kinc_display_count_available_modes(int display) {
	return 1;
}

bool kinc_display_available(int display) {
	return true;
}

const char *kinc_display_name(int display) {
	return "Display";
}

kinc_display_mode_t kinc_display_current_mode(int display) {
    kinc_display_mode_t mode;
    mode.x = 0;
    mode.y = 0;
#ifdef KORE_OPENGL
    mode.width = XWidthOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
    mode.width = 1920;
#endif
#ifdef KORE_OPENGL
    mode.height = XHeightOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
    mode.height = 1080;
#endif
    mode.frequency = 60;
    mode.bits_per_pixel = 32;
    mode.pixels_per_inch = 96;
    return mode;
}
