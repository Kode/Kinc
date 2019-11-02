#include "pch.h"

#include <kinc/display.h>
#include <kinc/log.h>

#include <X11/Xatom.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include <stdlib.h>

#ifdef KORE_OPENGL
#include <Kore/ogl.h>
#endif

void enumDisplayMonitors(kinc_display_t *displays, int& displayCounter);
#define MAXIMUM_DISPLAY_COUNT 10
static kinc_display_t displays[MAXIMUM_DISPLAY_COUNT];
static int displayCounter = -1;
static bool initialized = false;

extern "C" void enumerateDisplays() {
    if (initialized) {
        return;
    }

    initialized = true;
#ifdef KORE_OPENGL
    enumDisplayMonitors(displays, displayCounter);
#endif
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

int kinc_primary_display(void) {
	for (int i = 0; i < kinc_count_displays(); ++i) {
		if (displays[i].primary) {
			return i;
		}
	}
	return 0;
}

int kinc_count_displays(void) {
	return displayCounter + 1;
}

int width(int index) {
	return displays[index].width;
}

int height(int index) {
	return displays[index].height;
}

int x(int index) {
	return displays[index].x;
}

int y(int index) {
	return displays[index].y;
}

bool isPrimary(int index) {
	return displays[index].primary;
}

const kinc_display_t *primaryScreen() {
	for (int index = 0; index < MAXIMUM_DISPLAY_COUNT; ++index) {
		const kinc_display_t& info = displays[index];

		if (info.available && info.primary) {
			return &info;
		}
	}

	if (!displays[0].available) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "No display attached?");
		// TODO (DK) throw exception?
		return nullptr;
	}

	kinc_log(KINC_LOG_LEVEL_WARNING, "No primary display defined, returning first display");
	return &displays[0];
}

const kinc_display_t *screenById(int id) {
	for (int index = 0; index < MAXIMUM_DISPLAY_COUNT; ++index) {
		const kinc_display_t& info = displays[index];

		if (info.number == id) {
			return &info;
		}
	}

	if (!displays[0].available) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "No display available");
		// TODO (DK) throw exception?
		return nullptr;
	}

	kinc_log(KINC_LOG_LEVEL_WARNING, "No display with id \"%i\" found, returning first display", id);
	return &displays[0];
}
