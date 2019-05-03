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
	Kinc_WindowOptions kwin;
	if (win != nullptr) {
		kwin = convert(win);
	}
	
	Kinc_FramebufferOptions kframe;
	if (frame != nullptr) {
		kframe = convert(frame);
	}

	int index = Kinc_WindowCreate(win == nullptr ? nullptr : &kwin, frame == nullptr ? nullptr : &kframe);
	windows[index]._index = index;
	return &windows[index];
}

void Window::destroy(Window *window) {
	Kinc_WindowDestroy(window->_index);
}

Window *Window::get(int index) {
	return &windows[index];
}

int Window::count() {
	return Kinc_CountWindows();
}

void Window::resize(int width, int height) {
	Kinc_WindowResize(_index, width, height);
}

void Window::move(int x, int y) {
	Kinc_WindowMove(_index, x, y);
}

void Window::changeWindowMode(WindowMode mode) {
	Kinc_WindowChangeMode(_index, convert(mode));
}

void Window::changeWindowFeatures(int features) {
	Kinc_WindowChangeFeatures(_index, features);
}

void Window::changeFramebuffer(FramebufferOptions *frame) {
	Kinc_FramebufferOptions kframe = convert(frame);
	Kinc_WindowChangeFramebuffer(_index, &kframe);
}

int Window::x() {
	return Kinc_WindowX(_index);
}

int Window::y() {
	return Kinc_WindowY(_index);
}

int Window::width() {
	return Kinc_WindowWidth(_index);
}

int Window::height() {
	return Kinc_WindowHeight(_index);
}

Display *Window::display() {
	return Display::get(Kinc_WindowDisplay(_index));
}

WindowMode Window::mode() {
	switch (Kinc_WindowGetMode(_index)) {
	case KINC_WINDOW_MODE_WINDOW:
		return WindowModeWindow;
	case KINC_WINDOW_MODE_FULLSCREEN:
		return WindowModeFullscreen;
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
		return WindowModeExclusiveFullscreen;
	}
	return WindowModeWindow;
}

void Window::show() {
	Kinc_WindowShow(_index);
}

void Window::hide() {
	Kinc_WindowHide(_index);
}

void Window::setTitle(const char *title) {
	Kinc_WindowSetTitle(_index, title);
}

void Window::setResizeCallback(void(*callback)(int x, int y, void *data), void *data) {
	Kinc_WindowSetResizeCallback(_index, callback, data);
}

void Window::setPpiChangedCallback(void(*callback)(int ppi, void *data), void *data) {
	Kinc_WindowSetPpiChangedCallback(_index, callback, data);
}

bool Window::vSynced() {
	return kinc_window_vsynced(_index);
}

Window::Window() {

}
