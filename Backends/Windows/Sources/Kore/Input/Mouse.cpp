#include "../pch.h"

#include <Kore/Input/Mouse.h>
#include <Kore/System.h>

#include <Windows.h>

using namespace Kore;

void Mouse::_lock(int windowId, bool truth){
	show(!truth);
	if (truth){
		HWND hwnd = (HWND)Kore::System::windowHandle(windowId);
		SetCapture(hwnd);
		RECT rect;
		GetWindowRect(hwnd, &rect);
		ClipCursor(&rect);
	}
	else{
		ReleaseCapture();
		ClipCursor(NULL);
	}
}


bool Mouse::canLock(int windowId){
	return true;
}


void Mouse::show(bool truth){
	ShowCursor(truth);
}

void Mouse::setPosition(int windowId, int x, int y){
	HWND hwnd = (HWND)Kore::System::windowHandle(windowId);
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(hwnd, &point);
	SetCursorPos(point.x, point.y);
}

void Mouse::getPosition(int windowId, int& x, int& y){
	POINT point;
	GetCursorPos(&point);
	HWND hwnd = (HWND)Kore::System::windowHandle(windowId);
	ScreenToClient(hwnd, &point);
	x = point.x;
	y = point.y;
}
