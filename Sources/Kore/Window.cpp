#include "pch.h"

#include "Window.h"

#include "Convert.h"
#include "Display.h"

#include <Kinc/Window.h>

using namespace Kore;

namespace {
	const int MAXIMUM_WINDOWS = 16;
	Window windows[MAXIMUM_WINDOWS];
}

Window *Window::create(WindowOptions *win, FramebufferOptions *frame) {
	Kore_WindowOptions kwin;
	if (win != nullptr) {
		kwin = convert(win);
	}
	
	Kore_FramebufferOptions kframe;
	if (frame != nullptr) {
		kframe = convert(frame);
	}

	int index = Kore_WindowCreate(win == nullptr ? nullptr : &kwin, frame == nullptr ? nullptr : &kframe);
	windows[index]._index = index;
	return &windows[index];
}

void Window::destroy(Window *window) {
	Kore_WindowDestroy(window->_index);
}

Window *Window::get(int index) {
	return &windows[index];
}

int Window::count() {
	return Kore_CountWindows();
}

void Window::resize(int width, int height) {
	Kore_WindowResize(_index, width, height);
}

void Window::move(int x, int y) {
	Kore_WindowMove(_index, x, y);
}

void Window::changeWindowMode(WindowMode mode) {
	Kore_WindowChangeMode(_index, convert(mode));
}

void Window::changeWindowFeatures(int features) {
	Kore_WindowChangeFeatures(_index, features);
}

void Window::changeFramebuffer(FramebufferOptions *frame) {
	Kore_FramebufferOptions kframe = convert(frame);
	Kore_WindowChangeFramebuffer(_index, &kframe);
}

int Window::x() {
	return Kore_WindowX(_index);
}

int Window::y() {
	return Kore_WindowY(_index);
}

int Window::width() {
	return Kore_WindowWidth(_index);
}

int Window::height() {
	return Kore_WindowHeight(_index);
}

Display *Window::display() {
	return Display::get(Kore_WindowDisplay(_index));
}

WindowMode Window::mode() {
	switch (Kore_WindowGetMode(_index)) {
	case WINDOW_MODE_WINDOW:
		return WindowModeWindow;
	case WINDOW_MODE_FULLSCREEN:
		return WindowModeFullscreen;
	case WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
		return WindowModeExclusiveFullscreen;
	}
	return WindowModeWindow;
}

void Window::show() {
	Kore_WindowShow(_index);
}

void Window::hide() {
	Kore_WindowHide(_index);
}

void Window::setTitle(const char *title) {
	Kore_WindowSetTitle(_index, title);
}

void Window::setResizeCallback(void(*callback)(int x, int y, void *data), void *data) {
	Kore_WindowSetResizeCallback(_index, callback, data);
}

void Window::setPpiChangedCallback(void(*callback)(int ppi, void *data), void *data) {
	Kore_WindowSetPpiChangedCallback(_index, callback, data);
}

bool Window::vSynced() {
	return Kore_WindowVSynced(_index);
}

Window::Window() {

}
