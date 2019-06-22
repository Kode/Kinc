#include "pch.h"

#import <Cocoa/Cocoa.h>

#include <kinc/display.h>
#include <kinc/log.h>

using namespace Kore;

namespace {
	const int maxDisplays = 10;
	//Display displays[maxDisplays];
}

/*void initMacDisplays() {
	for (int i = 0; i < maxDisplays; ++i) {
		displays[i]._data.index = i;
	}
}*/

int kinc_count_displays() {
	NSArray* screens = [NSScreen screens];
	return (int)[screens count];
}

int kinc_primary_display() {
	NSArray* screens = [NSScreen screens];
	NSScreen* mainScreen = [NSScreen mainScreen];
	for (int i = 0; i < maxDisplays; ++i) {
		if (mainScreen == screens[i]) {
			return i;
		}
	}
	return -1;
}

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
	kinc_display_mode_t dm;
	dm.width = 800;
	dm.height = 600;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;
	return dm;
}

int kinc_display_count_available_modes(int display) {
	return 1;
}

bool kinc_display_available(int display) {
	return true;
}

const char *kinc_display_name(int display) {
	return "Display";
}

kinc_display_mode_t kinc_display_current_mode(int display) {
	kinc_display_mode_t dm;
	dm.width = 800;
	dm.height = 600;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;
	return dm;
}

//**
/*
int Display::x() {
	NSArray* screens = [NSScreen screens];
	NSScreen* screen = screens[_data.index];
	NSRect rect = [screen frame];
	return rect.origin.x;
}

int Display::y() {
	NSArray* screens = [NSScreen screens];
	NSScreen* screen = screens[_data.index];
	NSRect rect = [screen frame];
	return rect.origin.y;
}

int Display::width() {
	NSArray* screenArray = [NSScreen screens];
	NSScreen* screen = [screenArray objectAtIndex:_data.index];
	NSRect screenRect = [screen visibleFrame];
	return screenRect.size.width;
}

int Display::height() {
	NSArray* screenArray = [NSScreen screens];
	// unsigned screenCount = [screenArray count];
	NSScreen* screen = [screenArray objectAtIndex:_data.index];
	NSRect screenRect = [screen visibleFrame];
	return screenRect.size.height;
}

int Display::frequency() {
	return 60;
}

int Display::pixelsPerInch() {
	return 96;
}
*/
