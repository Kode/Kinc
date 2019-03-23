#include "pch.h"

#import <Cocoa/Cocoa.h>

#include <Kinc/Input/Mouse.h>
#include <Kinc/Window.h>

void Kinc_Internal_Mouse_Lock(int window) {
	Kinc_Mouse_Hide();
}

void Kinc_Internal_Mouse_Unlock(int window) {
	Kinc_Mouse_Show();
}

bool Kinc_Mouse_CanLock(int window) {
	return true;
}

void Kinc_Mouse_Show() {
	CGDisplayShowCursor(kCGDirectMainDisplay);
}

void Kinc_Mouse_Hide() {
	CGDisplayHideCursor(kCGDirectMainDisplay);
}

void Kinc_Mouse_SetPosition(int windowId, int x, int y) {
	//**
	/*NSWindow* window = Window::get(windowId)->_data.handle;
	NSRect rect = [[NSScreen mainScreen] frame];
	
	// Flip y and add window offset
	CGPoint point;
	point.x = x + window.frame.origin.x;
	point.y = rect.size.height - (y + window.frame.origin.y);
	
	CGWarpMouseCursorPosition(point);
	CGAssociateMouseAndMouseCursorPosition(true);*/
}

void Kinc_Mouse_GetPosition(int windowId, int& x, int& y) {
	//**
	/*NSWindow* window = Window::get(windowId)->_data.handle;
	NSRect rect = [[NSScreen mainScreen] frame];
	CGEventRef event = CGEventCreate(NULL);
	CGPoint point = CGEventGetLocation(event);
	CFRelease(event);

	// Flip y and remove window offset
	x = point.x - window.frame.origin.x;
	y = rect.size.height - (point.y + window.frame.origin.y);*/
}
