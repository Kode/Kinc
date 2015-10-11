#include "../pch.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Graphics/Graphics.h>
#include <Windows.h>

using namespace Kore;

void Mouse::_lock(bool truth){
	show(!truth);
	if (truth){
		auto hwnd = (HWND)Kore::Graphics::getControl();
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


bool Mouse::canLock(){
	return true;
}


void Mouse::show(bool truth){
	ShowCursor(truth);
}

void Mouse::setPosition(int x, int y){
	auto hwnd = (HWND)Kore::Graphics::getControl();
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(hwnd, &point);
	SetCursorPos(point.x, point.y);
}

void Mouse::getPosition(int& x, int& y){
	POINT point;
	GetCursorPos(&point);
	auto hwnd = (HWND)Kore::Graphics::getControl();
	ScreenToClient(hwnd, &point);
	x = point.x;
	y = point.y;
}
