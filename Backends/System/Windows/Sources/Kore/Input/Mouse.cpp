#include "../pch.h"

#include <Kore/Input/Mouse.h>
#include <Kore/System.h>
#include <Kore/Window.h>

#include <Windows.h>

using namespace Kore;

void Mouse::_lock(int windowId, bool truth) {
	show(!truth);
	if (truth) {
		HWND handle = Kore::Window::get(windowId)->_data.handle;
		SetCapture(handle);
		RECT rect;
		GetWindowRect(handle, &rect);
		ClipCursor(&rect);
	}
	else {
		ReleaseCapture();
		ClipCursor(nullptr);
	}
}

bool Mouse::canLock(int windowId) {
	return true;
}

void Mouse::show(bool truth) {
	ShowCursor(truth);
}

void Mouse::setPosition(int windowId, int x, int y) {
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(Window::get(windowId)->_data.handle, &point);
	SetCursorPos(point.x, point.y);
}

void Mouse::getPosition(int windowId, int& x, int& y) {
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(Window::get(windowId)->_data.handle, &point);
	x = point.x;
	y = point.y;
}
