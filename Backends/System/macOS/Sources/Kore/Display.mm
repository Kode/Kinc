#include "pch.h"

#import <Cocoa/Cocoa.h>

#include <Kinc/Display.h>
#include <Kinc/Log.h>

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

int Kinc_CountDisplays() {
	NSArray* screens = [NSScreen screens];
	return (int)[screens count];
}

int Kinc_PrimaryDisplay() {
	NSArray* screens = [NSScreen screens];
	NSScreen* mainScreen = [NSScreen mainScreen];
	for (int i = 0; i < maxDisplays; ++i) {
		if (mainScreen == screens[i]) {
			return i;
		}
	}
	return -1;
}

Kinc_DisplayMode Kinc_DisplayAvailableMode(int display, int mode) {
	Kinc_DisplayMode dm;
	dm.width = 800;
	dm.height = 600;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;
	return dm;
}

int Kinc_DisplayCountAvailableModes(int display) {
	return 1;
}

bool Kinc_DisplayAvailable(int display) {
	return true;
}

const char *Kinc_DisplayName(int display) {
	return "Display";
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
