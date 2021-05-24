#import <Cocoa/Cocoa.h>

#include <kinc/input/mouse.h>
#include <kinc/window.h>

void kinc_internal_mouse_lock(int window) {
	kinc_mouse_hide();
}

void kinc_internal_mouse_unlock(void) {
	kinc_mouse_show();
}

bool kinc_mouse_can_lock(void) {
	return true;
}

void kinc_mouse_show() {
	CGDisplayShowCursor(kCGDirectMainDisplay);
}

void kinc_mouse_hide() {
	CGDisplayHideCursor(kCGDirectMainDisplay);
}

void kinc_mouse_set_position(int windowId, int x, int y) {
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

void kinc_mouse_get_position(int windowId, int* x, int* y) {
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

void kinc_mouse_set_cursor(int cursor_index) {

}
