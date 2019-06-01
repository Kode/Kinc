#include "pch.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/input/mouse.h>
#include <kinc/system.h>

namespace {
	int mouseX, mouseY;
	bool keyboardShown = false;
}

void Kinc_Mouse_GetPosition(int window, int *x, int *y) {
	*x = mouseX;
	*y = mouseY;
}

void Kinc_ShowKeyboard() {
	keyboardShown = true;
}

void Kinc_HideKeyboard() {
	keyboardShown = false;
}

bool Kinc_ShowsKeyboard() {
	return keyboardShown;
}

void Kinc_loadURL(const char* url) {}

void kinc_vibrate(int ms) {}

const char* kinc_language() {
	return "en";
}

const char* Kinc_SystemId() {
	return "OSX";
}

namespace {
	const char* videoFormats[] = {"ogv", nullptr};
}

const char** Kinc_VideoFormats() {
	return ::videoFormats;
}

void Kinc_SetKeepScreenOn(bool on) {}

#include <mach/mach_time.h>

double kinc_frequency() {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

kinc_ticks_t kinc_timestamp() {
	return mach_absolute_time();
}
