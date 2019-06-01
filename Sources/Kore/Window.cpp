#include "pch.h"

#include "Window.h"

#include "Convert.h"
#include "Display.h"

#include <kinc/window.h>

using namespace Kore;

namespace {
	const int MAXIMUM_WINDOWS = 16;
	Window windows[MAXIMUM_WINDOWS];
}

Window *Window::create(WindowOptions *win, FramebufferOptions *frame) {
	kinc_window_options_t kwin;
	if (win != nullptr) {
		kwin = convert(win);
	}
	
	kinc_framebuffer_options_t kframe;
	if (frame != nullptr) {
		kframe = convert(frame);
	}

	int index = kinc_window_create(win == nullptr ? nullptr : &kwin, frame == nullptr ? nullptr : &kframe);
	windows[index]._index = index;
	return &windows[index];
}

void Window::destroy(Window *window) {
	kinc_window_destroy(window->_index);
}

Window *Window::get(int index) {
	return &windows[index];
}

int Window::count() {
	return kinc_count_windows();
}

void Window::resize(int width, int height) {
	kinc_window_resize(_index, width, height);
}

void Window::move(int x, int y) {
	kinc_window_move(_index, x, y);
}

void Window::changeWindowMode(WindowMode mode) {
	kinc_window_change_mode(_index, convert(mode));
}

void Window::changeWindowFeatures(int features) {
	kinc_window_change_features(_index, features);
}

void Window::changeFramebuffer(FramebufferOptions *frame) {
	kinc_framebuffer_options_t kframe = convert(frame);
	kinc_window_change_framebuffer(_index, &kframe);
}

int Window::x() {
	return kinc_window_x(_index);
}

int Window::y() {
	return kinc_window_y(_index);
}

int Window::width() {
	return kinc_window_width(_index);
}

int Window::height() {
	return kinc_window_height(_index);
}

Display *Window::display() {
	return Display::get(kinc_window_display(_index));
}

WindowMode Window::mode() {
	switch (kinc_window_get_mode(_index)) {
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
	kinc_window_show(_index);
}

void Window::hide() {
	kinc_window_hide(_index);
}

void Window::setTitle(const char *title) {
	kinc_window_set_title(_index, title);
}

void Window::setResizeCallback(void(*callback)(int x, int y, void *data), void *data) {
	kinc_window_set_resize_callback(_index, callback, data);
}

void Window::setPpiChangedCallback(void(*callback)(int ppi, void *data), void *data) {
	kinc_window_set_ppi_changed_callback(_index, callback, data);
}

bool Window::vSynced() {
	return kinc_window_vsynced(_index);
}

Window::Window() {

}
