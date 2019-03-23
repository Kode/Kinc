#include "pch.h"

#include "Display.h"

#include <Kinc/Display.h>

using namespace Kore;

namespace {
	const int MAXIMUM_DISPLAYS = 16;
	Display displays[MAXIMUM_DISPLAYS];
}

Display* Display::primary() {
	displays[Kinc_PrimaryDisplay()]._index = Kinc_PrimaryDisplay();
	return &displays[Kinc_PrimaryDisplay()];
}

Display* Display::get(int index) {
	displays[index]._index = index;
	return &displays[index];
}

int Display::count() {
	return Kinc_CountDisplays();
}

bool Display::available() {
	return Kinc_DisplayAvailable(_index);
}

const char* Display::name() {
	return Kinc_DisplayName(_index);
}

int Display::x() {
	return Kinc_DisplayCurrentMode(_index).x;
}

int Display::y() {
	return Kinc_DisplayCurrentMode(_index).y;
}

int Display::width() {
	return Kinc_DisplayCurrentMode(_index).width;
}

int Display::height() {
	return Kinc_DisplayCurrentMode(_index).height;
}

int Display::frequency() {
	return Kinc_DisplayCurrentMode(_index).frequency;
}

int Display::pixelsPerInch() {
	return Kinc_DisplayCurrentMode(_index).pixels_per_inch;
}

DisplayMode Display::availableMode(int index) {
	DisplayMode mode;
	Kinc_DisplayMode kMode = Kinc_DisplayAvailableMode(_index, index);
	mode.width = kMode.width;
	mode.height = kMode.height;
	mode.frequency = kMode.frequency;
	mode.bitsPerPixel = kMode.bits_per_pixel;
	return mode;
}

int Display::countAvailableModes() {
	return Kinc_DisplayCountAvailableModes(_index);
}

Display::Display() {}
