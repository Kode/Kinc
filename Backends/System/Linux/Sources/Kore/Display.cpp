#include "pch.h"

#include <kinc/display.h>
#include <kinc/log.h>

#include <X11/Xatom.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <stdlib.h>

typedef struct {
	bool available;
	int x;
	int y;
	int width;
	int height;
	bool primary;
	int number;
} kinc_display_t;

void enumDisplayMonitors(kinc_display_t *displays, int& displayCounter);
#define MAXIMUM_DISPLAY_COUNT 10
static kinc_display_t displays[MAXIMUM_DISPLAY_COUNT];
static int displayCounter = -1;
static bool display_initialized = false;

void kinc_display_init() {
    if (display_initialized) {
        return;
    }
    enumDisplayMonitors(displays, displayCounter);
    display_initialized = true;
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
	Display *disp = XOpenDisplay(NULL);
	mode.width = XWidthOfScreen(XDefaultScreenOfDisplay(disp));
	mode.height = XHeightOfScreen(XDefaultScreenOfDisplay(disp));
	mode.frequency = 60;
	Window win = RootWindow(disp, DefaultScreen(disp));
	XRRScreenResources *res = XRRGetScreenResourcesCurrent(disp, win);
	RROutput primary = XRRGetOutputPrimary(disp, win);
	XRROutputInfo *output_info = NULL;

	if (primary != 0) {
		output_info = XRRGetOutputInfo(disp, res, primary);

		if (output_info->connection != RR_Connected || output_info->crtc == 0) {
			XRRFreeOutputInfo(output_info);
			output_info = NULL;
		}
	}

	if (output_info == NULL) {
		for (int i = 0; i < res->noutput; ++i) {
			output_info = XRRGetOutputInfo(disp, res, res->outputs[i]);

			if (output_info->connection != RR_Connected || output_info->crtc == 0) {
				XRRFreeOutputInfo(output_info);
				output_info = NULL;
			} else {
				break;
			}
		}
	}

	if (output_info != NULL) {
		XRRCrtcInfo *crtc = XRRGetCrtcInfo(disp, res, output_info->crtc);
		for (int j = 0; j < res->nmode; ++j) {
			XRRModeInfo *mode_info = &res->modes[j];
			if (crtc->mode == mode_info->id) {
				if (mode_info->hTotal && mode_info->vTotal) {
					mode.frequency = (mode_info->dotClock / (mode_info->hTotal * mode_info->vTotal));
				}
				break;
			}
		}
		XRRFreeCrtcInfo(crtc);
		XRRFreeOutputInfo(output_info);
	}

	XRRFreeScreenResources(res);
	XCloseDisplay(disp);
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
		return nullptr;
	}

	kinc_log(KINC_LOG_LEVEL_WARNING, "No display with id \"%i\" found, returning first display", id);
	return &displays[0];
}
