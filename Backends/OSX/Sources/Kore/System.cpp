#include "pch.h"
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>

using namespace Kore;

namespace {
	int mouseX, mouseY;
	bool keyboardShown = false;
}

vec2i System::mousePos() {
	return vec2i(mouseX, mouseY);
}

void System::changeResolution(int width, int height, bool fullscreen) {
	
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

void System::loadURL(const char* url) {
	
}

const char* System::systemId() {
	return "OSX";
}

namespace {
	const char* videoFormats[] = { "ogv", nullptr };
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
}

void System::showWindow() {
	
}

void System::setTitle(const char* title) {
	
}

void System::setKeepScreenOn( bool on ) {
    
}

#include <mach/mach_time.h>

double System::frequency() {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

System::ticks System::timestamp() {
	return mach_absolute_time();
}

namespace appstate {
    int currentDeviceId = -1;
}

void Kore::System::setup() {    
}

bool Kore::System::isFullscreen() {
    return false; // TODO (DK)
}

int Kore::System::currentDevice() {
    return appstate::currentDeviceId;
}

void Kore::System::makeCurrent(int contextId) {
    appstate::currentDeviceId = contextId;
    Graphics::makeCurrent(contextId);
}

void Kore::System::clearCurrent() {
    appstate::currentDeviceId = -1;
}