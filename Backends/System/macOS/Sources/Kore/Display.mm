#include "pch.h"

#import <Cocoa/Cocoa.h>

#include <Kore/Display.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	const int maxDisplays = 10;
	Display displays[maxDisplays];
}

void initMacDisplays() {
	for (int i = 0; i < maxDisplays; ++i) {
		displays[i]._data.index = i;
	}
}

int Display::count() {
	NSArray* screens = [NSScreen screens];
	return (int)[screens count];
}

Display* Display::primary() {
	NSArray* screens = [NSScreen screens];
	NSScreen* mainScreen = [NSScreen mainScreen];
	for (int i = 0; i < maxDisplays; ++i) {
		if (mainScreen == screens[i]) {
			return &displays[i];
		}
	}
	return nullptr;
}

Display* Display::get(int index) {
	return &displays[index];
}

DisplayMode Display::availableMode(int index) {
	DisplayMode mode;
	mode.width = 800;
	mode.height = 600;
	mode.frequency = 60;
	mode.bitsPerPixel = 32;
	return mode;
}

int Display::countAvailableModes() {
	return 1;
}

int Display::pixelsPerInch() {
	return 96;
}

DisplayData::DisplayData() {}

bool Display::available() {
	return true;
}

const char* Display::name() {
	return "Display";
}

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

Display::Display() {
	
}
