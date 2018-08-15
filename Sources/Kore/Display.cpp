#include "pch.h"

#include "Display.h"

#include "../C/Kore/Display.h"

using namespace Kore;

namespace {
	const int MAXIMUM_DISPLAYS = 16;
	Display displays[MAXIMUM_DISPLAYS];
}

Display* Display::primary() {
	displays[Kore_PrimaryDisplay()]._index = Kore_PrimaryDisplay();
	return &displays[Kore_PrimaryDisplay()];
}

Display* Display::get(int index) {
	displays[index]._index = index;
	return &displays[index];
}

int Display::count() {
	return Kore_CountDisplays();
}

bool Display::available() {
	return Kore_DisplayAvailable(_index);
}

const char* Display::name() {
	return Kore_DisplayName(_index);
}

int Display::x() {
	return Kore_DisplayCurrentMode(_index).x;
}

int Display::y() {
	return Kore_DisplayCurrentMode(_index).y;
}

int Display::width() {
	return Kore_DisplayCurrentMode(_index).width;
}

int Display::height() {
	return Kore_DisplayCurrentMode(_index).height;
}

int Display::frequency() {
	return Kore_DisplayCurrentMode(_index).frequency;
}

int Display::pixelsPerInch() {
	return Kore_DisplayCurrentMode(_index).pixels_per_inch;
}

DisplayMode Display::availableMode(int index) {
	DisplayMode mode;
	Kore_DisplayMode kMode = Kore_DisplayAvailableMode(_index, index);
	mode.width = kMode.width;
	mode.height = kMode.height;
	mode.frequency = kMode.frequency;
	mode.bitsPerPixel = kMode.bits_per_pixel;
	return mode;
}

int Display::countAvailableModes() {
	return Kore_DisplayCountAvailableModes(_index);
}

Display::Display() {}
