#include <Kore/Graphics/Graphics.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#include "Display.h"

using namespace Kore;

bool Kore::System::handleMessages() {
	return true;
}

vec2i Kore::System::mousePos() {
	return vec2i(0, 0);
}

int createWindow(const char *title, int x, int y, int width, int height, WindowMode windowMode, int targetDisplay) {
	return 0;
}

int Kore::System::windowWidth(int window) {
	return 100;
}

int Kore::System::windowHeight(int window) {
	return 100;
}

void *Kore::System::windowHandle(int windowId) {
	return nullptr;
}

void Kore::System::destroyWindow(int index) {}

void Kore::System::makeCurrent(int contextId) {}

void Kore::System::clearCurrent() {}

int Kore::System::initWindow(WindowOptions options) {
	return 0;
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {}

void Kore::System::setup() {}

bool Kore::System::isFullscreen() {
	return false;
}

void Kore::System::showKeyboard() {}

void Kore::System::hideKeyboard() {}

bool Kore::System::showsKeyboard() {
	return true;
}

void Kore::System::loadURL(const char *url) {}

void Kore::System::vibrate(int ms) {}

const char *Kore::System::language() {
	return "en";
}

void Kore::System::setTitle(const char *title) {}

void Kore::System::setKeepScreenOn(bool on) {}

void Kore::System::showWindow() {}

const char *Kore::System::systemId() {
	return "Windows";
}

const char *Kore::System::savePath() {
	return "";
}

const char **Kore::System::videoFormats() {
	return nullptr;
}

double Kore::System::frequency() {
	return 1;
}

Kore::System::ticks Kore::System::timestamp() {
	return 1;
}
