#include "pch.h"
#include <Kore/Graphics4/Graphics.h>
#include <Kore/System.h>

using namespace Kore;

namespace {
	int mouseX, mouseY;
	bool keyboardShown = false;
}

vec2i System::mousePos() {
	return vec2i(mouseX, mouseY);
}

void System::showKeyboard() {
	keyboardShown = true;
}

void System::hideKeyboard() {
	keyboardShown = false;
}

bool System::showsKeyboard() {
	return keyboardShown;
}

void System::loadURL(const char* url) {}

void Kore::System::vibrate(int ms) {}

const char* Kore::System::language() {
	return "en";
}

const char* System::systemId() {
	return "OSX";
}

namespace {
	const char* videoFormats[] = {"ogv", nullptr};
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
}

void System::setKeepScreenOn(bool on) {}

#include <mach/mach_time.h>

double System::frequency() {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

System::ticks System::timestamp() {
	return mach_absolute_time();
}
