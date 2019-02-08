#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Window.h>

#include <string.h>

namespace {
	Kore::Window win;
}

Kore::Window* Kore::Window::get(int window) {
	if (window > 0) {
		return nullptr;
	}
	return &win;
}

int Kore::Window::count() {
	return 1;
}

int Kore::Window::x() {
	return 0;
}

int Kore::Window::y() {
	return 0;
}

int Kore::Window::width() {
    return _data.width;
}

int Kore::Window::height() {
    return _data.height;
}

void Kore::Window::resize(int width, int height) {}

void Kore::Window::move(int x, int y) {}

void Kore::Window::changeFramebuffer(FramebufferOptions* frame) {
	Graphics4::_changeFramebuffer(0, frame);
}

void Kore::Window::changeWindowFeatures(int features) {}

//In HTML5 fullscreen is activable only from user input.
void Kore::Window::changeWindowMode(WindowMode mode) {
	if (mode == WindowModeFullscreen || mode == WindowModeExclusiveFullscreen) {
		if (_data.mode == WindowModeFullscreen || _data.mode == WindowModeExclusiveFullscreen) {
			_data.mode = mode;
			return;
		}
		//TODO: call js Fullscreen API
		_data.mode = mode;
	}
	else {
		if (mode == _data.mode) {
			return;
		}
		//TODO: call js Fullscreen API
        _data.mode = mode;
	}
}

Kore::Display* Kore::Window::display() {
	return nullptr;
}

void Kore::Window::destroy(Window* window) {

}

void Kore::Window::show() {}

void Kore::Window::hide() {}

//TODO: change browser title.
void Kore::Window::setTitle(const char* title) {}

Kore::Window* Kore::Window::create(WindowOptions* win, FramebufferOptions* frame) {
	return nullptr;
}
Kore::WindowData::WindowData() : mode(0) {}

Kore::Window::Window() {}

void Kore::Window::setResizeCallback(void (*value)(int x, int y, void* data), void* data) {}

void Kore::Window::setPpiChangedCallback(void(*callback)(int ppi, void* data), void* data) {}

Kore::WindowMode Kore::Window::mode() {
	return (WindowMode)_data.mode;
}