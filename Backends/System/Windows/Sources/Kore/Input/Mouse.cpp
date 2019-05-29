#include "../pch.h"

#include <Kore/System.h>
#include <Kore/Window.h>
#include <Kore/Windows.h>

#include <Kinc/Input/Mouse.h>

#include <Windows.h>

using namespace Kore;

void Kinc_Internal_Mouse_Lock(int window) {
	Kinc_Mouse_Hide();
	HWND handle = Kinc_Windows_WindowHandle(window);
	SetCapture(handle);
	RECT rect;
	GetWindowRect(handle, &rect);
	ClipCursor(&rect);
}

void Kinc_Internal_Mouse_Unlock(int window) {
	Kinc_Mouse_Show();
	ReleaseCapture();
	ClipCursor(nullptr);
}

bool Kinc_Mouse_CanLock(int window) {
	return true;
}

void Kinc_Mouse_Show() {
	 the internal counter of ShowCursor
	if (truth)
		while (ShowCursor(truth) < 0);
	else
		while (ShowCurso
}

void Kinc_Mouse_Hide() {
	ShowCursor(false);
}

void Kinc_Mouse_SetPosition(int window, int x, int y) {
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(Kinc_Windows_WindowHandle(window), &point);
	SetCursorPos(point.x, point.y);
}

void Kinc_Mouse_GetPosition(int window, int *x, int *y) {
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(Kinc_Windows_WindowHandle(window), &point);
	*x = point.x;
	*y = point.y;
}
