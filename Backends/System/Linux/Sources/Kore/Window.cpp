#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Window.h>
#include <Kore/Linux.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <string.h>

namespace {
	Kore::Window win;
}

/*Window* Window::get(int window) {
	if (window > 0) {
		return nullptr;
	}
	return &win;
}*/

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

void Kore::Window::resize(int width, int height) {

}

void Kore::Window::move(int x, int y) {

}

void Kore::Window::changeFramebuffer(FramebufferOptions* frame) {
	Graphics4::_changeFramebuffer(0, frame);
}

void Kore::Window::changeWindowFeatures(int features) {

}

void Kore::Linux::fullscreen(XID window, bool value) {
	Atom wm_state = XInternAtom(Kore::Linux::display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(Kore::Linux::display, "_NET_WM_STATE_FULLSCREEN", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = value ? 1 : 0;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XMapWindow(Kore::Linux::display, window);

	XSendEvent(Kore::Linux::display, DefaultRootWindow(Kore::Linux::display), False,
			   SubstructureRedirectMask | SubstructureNotifyMask, &xev);

	XFlush(Kore::Linux::display);
}

void Kore::Window::changeWindowMode(WindowMode mode) {
	if (mode == WindowModeFullscreen || mode == WindowModeExclusiveFullscreen) {
		if (_data.mode == WindowModeFullscreen || _data.mode == WindowModeExclusiveFullscreen) {
			_data.mode = mode;
			return;
		}

		Kore::Linux::fullscreen(_data.handle, true);
		_data.mode = mode;
	}
	else {
		if (mode == _data.mode) {
			return;
		}

		Kore::Linux::fullscreen(_data.handle, false);
        _data.mode = mode;
	}
}

Kore::Display* Kore::Window::display() {
	return nullptr;
}

void Kore::Window::destroy(Window* window) {

}

void Kore::Window::show() {

}

void Kore::Window::hide() {

}

void Kore::Window::setTitle(const char* title) {

}

Kore::Window* Kore::Window::create(WindowOptions* win, FramebufferOptions* frame) {
	return nullptr;
}

Kore::WindowData::WindowData() : mode(0) {}

Kore::Window::Window() {}

void Kore::Window::setResizeCallback(void (*value)(int x, int y, void* data), void* data) {

}

void Kore::Window::setPpiChangedCallback(void(*callback)(int ppi, void* data), void* data) {
	
}

Kore::WindowMode Kore::Window::mode() {
	return (WindowMode)_data.mode;
}
