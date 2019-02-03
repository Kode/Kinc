#include "pch.h"

#import <Cocoa/Cocoa.h>

#include <Kore/Input/Mouse.h>
#include <Kore/Window.h>

using namespace Kore;

void Mouse::_lock(int windowId, bool truth) {
	show(!truth);
}

bool Mouse::canLock(int windowId) {
	return true;
}

void Mouse::show(bool truth) {
	truth ? CGDisplayShowCursor(kCGDirectMainDisplay) : CGDisplayHideCursor(kCGDirectMainDisplay);
}

void Mouse::setPosition(int windowId, int x, int y) {
	NSWindow* window = Window::get(windowId)->_data.handle;
	NSRect rect = [[NSScreen mainScreen] frame];
	
	// Flip y and add window offset
	CGPoint point;
	point.x = x + window.frame.origin.x;
	point.y = rect.size.height - (y + window.frame.origin.y);
	
	CGWarpMouseCursorPosition(point);
	CGAssociateMouseAndMouseCursorPosition(true);
}

void Mouse::getPosition(int windowId, int& x, int& y) {
	NSWindow* window = Window::get(windowId)->_data.handle;
	NSRect rect = [[NSScreen mainScreen] frame];
	CGEventRef event = CGEventCreate(NULL);
	CGPoint point = CGEventGetLocation(event);
	CFRelease(event);

	// Flip y and remove window offset
	x = point.x - window.frame.origin.x;
	y = rect.size.height - (point.y + window.frame.origin.y);
}
