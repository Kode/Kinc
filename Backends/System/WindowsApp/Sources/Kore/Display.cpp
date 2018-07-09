#include "pch.h"

#include <Kore/Display.h>

using namespace Kore;

namespace {
	Display display;
}

int Display::count() {
	return 1;
}

Display* Display::primary() {
	return &display;
}

Display* Display::get(int index) {
	if (index > 0) {
		return nullptr;
	}
	return &display;
}

DisplayMode Display::availableMode(int index) {
	DisplayMode mode;
	mode.width = 800;
	mode.height = 600;
	mode.frequency = 60;
	mode.bitsPerPixel = 32;
	return mode;
}

int Display::countAvailableModes() {
	return 1;
}

int Display::pixelsPerInch() {
	return 96;
}

DisplayData::DisplayData() {}

bool Display::available() {
	return true;
}

const char* Display::name() {
	return "Display";
}

int Display::x() {
	return 0;
}

int Display::y() {
	return 0;
}

int Display::width() {
	return 800;
}

int Display::height() {
	return 600;
}

int Display::frequency() {
	return 60;
}

Display::Display() {

}
