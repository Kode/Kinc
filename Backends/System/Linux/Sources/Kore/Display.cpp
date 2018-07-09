#include "pch.h"

#include <Kore/Display.h>

#include <X11/Xatom.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include <stdlib.h>

namespace {
	Kore::Display display;
}

int Kore::Display::count() {
	return 1;
}

Kore::Display* Kore::Display::primary() {
	return &display;
}

Kore::Display* Kore::Display::get(int index) {
	if (index > 0) {
		return nullptr;
	}
	return &display;
}

Kore::DisplayMode Kore::Display::availableMode(int index) {
	DisplayMode mode;
	mode.width = 800;
	mode.height = 600;
	mode.frequency = 60;
	mode.bitsPerPixel = 32;
	return mode;
}

int Kore::Display::countAvailableModes() {
	return 1;
}

int Kore::Display::pixelsPerInch() {
	return 96;
}

Kore::DisplayData::DisplayData() {}

bool Kore::Display::available() {
	return true;
}

const char* Kore::Display::name() {
	return "Display";
}

int Kore::Display::x() {
	return 0;
}

int Kore::Display::y() {
	return 0;
}

int Kore::Display::width() {
#ifdef KORE_OPENGL
	return XWidthOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
	return 1920;
#endif
}

int Kore::Display::height() {
#ifdef KORE_OPENGL
	return XHeightOfScreen(XDefaultScreenOfDisplay(XOpenDisplay(NULL)));
#else
	return 1080;
#endif
}

int Kore::Display::frequency() {
	return 60;
}

Kore::Display::Display() {

}
