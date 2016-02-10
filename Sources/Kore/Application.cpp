#include "pch.h"
#include "Application.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <limits>

using namespace Kore;

namespace {
	Application* instance = nullptr;
	int width, height;
	int x, y;
	int antialiasing;
	int windowMode, initialWindowMode;
	const char* name;
	bool showWindow;
}

Application::Application(int argc, char** argv, int width, int height, int antialiasing, int windowMode, const char* name, bool showWindow, int x, int y) : callback(nullptr), orientationCallback(nullptr), foregroundCallback(nullptr), resumeCallback(nullptr), pauseCallback(nullptr), backgroundCallback(nullptr), shutdownCallback(nullptr), running(false) {
	::width = width;
	::height = height;
	::antialiasing = antialiasing;
	::windowMode = ::initialWindowMode = windowMode;
	::name = name;
	::showWindow = showWindow;
	::x = x;
	::y = y;
	instance = this;
	Random::init(static_cast<int>(System::timestamp() % std::numeric_limits<int>::max()));
	Graphics::init();
}

Application::~Application() {
	Graphics::destroy();
}

void Application::start() {
	running = true;
#if !defined(SYS_HTML5) && !defined(SYS_TIZEN)
	// if (Graphics::hasWindow()) Graphics::swapBuffers();
	while (running) {
		callback();
		System::handleMessages();
	}
#endif
}

void Application::stop() {
	running = false;
}

const char* Application::name() {
	return ::name;
}

int Application::x() {
	return ::x;
}

int Application::y() {
	return ::y;
}

int Application::width() {
	return ::width;
}

int Application::height() {
	return ::height;
}

int Application::antialiasing() {
	return ::antialiasing;
}

int Application::windowMode() {
	return ::windowMode;
}

bool Application::fullscreen() {
	return ::windowMode == 2;
}

bool Application::showWindow() {
	return ::showWindow;
}

void Application::setWidth(int width) {
	::width = width;
}

void Application::setHeight(int height) {
	::height = height;
}

void Application::setFullscreen(bool fullscreen) {
	::windowMode = fullscreen ? 2 : ::initialWindowMode;
}

void Application::setCallback(void (*callback)()) {
	this->callback = callback;
}

Application* Application::the() {
	return instance;
}
