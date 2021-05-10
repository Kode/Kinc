#include "Display.h"

#include <kinc/display.h>

using namespace Kore;

namespace {
	const int MAXIMUM_DISPLAYS = 16;
	Display displays[MAXIMUM_DISPLAYS];
}

void Display::init() {
	kinc_display_init();
}

Display *Display::primary() {
	displays[kinc_primary_display()]._index = kinc_primary_display();
	return &displays[kinc_primary_display()];
}

Display *Display::get(int index) {
	displays[index]._index = index;
	return &displays[index];
}

int Display::count() {
	return kinc_count_displays();
}

bool Display::available() {
	return kinc_display_available(_index);
}

const char *Display::name() {
	return kinc_display_name(_index);
}

int Display::x() {
	return kinc_display_current_mode(_index).x;
}

int Display::y() {
	return kinc_display_current_mode(_index).y;
}

int Display::width() {
	return kinc_display_current_mode(_index).width;
}

int Display::height() {
	return kinc_display_current_mode(_index).height;
}

int Display::frequency() {
	return kinc_display_current_mode(_index).frequency;
}

int Display::pixelsPerInch() {
	return kinc_display_current_mode(_index).pixels_per_inch;
}

DisplayMode Display::availableMode(int index) {
	DisplayMode mode;
	kinc_display_mode_t kMode = kinc_display_available_mode(_index, index);
	mode.width = kMode.width;
	mode.height = kMode.height;
	mode.frequency = kMode.frequency;
	mode.bitsPerPixel = kMode.bits_per_pixel;
	return mode;
}

int Display::countAvailableModes() {
	return kinc_display_count_available_modes(_index);
}

Display::Display() {}
