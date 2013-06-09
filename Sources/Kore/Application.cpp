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
	bool full;
	const char* name;
	bool showWindow;
}

Application::Application(int argc, char** argv, int width, int height, bool fullscreen, const char* name, bool showWindow) : callback(nullptr), running(false) {
	::width = width;
	::height = height;
	::full = fullscreen;
	::name = name;
	::showWindow = showWindow;
	instance = this;
	Random::init(static_cast<int>(System::timestamp() % std::numeric_limits<int>::max()));
	Graphics::init();
}

Application::~Application() {
	Graphics::destroy();
}

void Application::start() {
	running = true;
#ifndef SYS_ANDROID
#ifndef SYS_HTML5
	if (Graphics::hasWindow()) Graphics::swapBuffers();
	while (running) {
		callback();
		System::handleMessages();
	}
#endif
#endif
}

void Application::stop() {
	running = false;
}

const char* Application::name() {
	return ::name;
}

int Application::width() {
	return ::width;
}

int Application::height() {
	return ::height;
}

bool Application::fullscreen() {
	return ::full;
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
	::full = fullscreen;
}

void Application::setCallback(void (*callback)()) {
	this->callback = callback;
}

Application* Application::the() {
	return instance;
}
