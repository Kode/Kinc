#include "pch.h"

#import <Cocoa/Cocoa.h>

#include "Display.h"
#include <Kore/Log.h>

namespace Kore {
	namespace Display {
		int count() {
			NSArray *screens = [NSScreen screens];
    		return [screens count];
		}

		int width(int index) {
			NSArray *screens = [NSScreen screens];
			NSScreen *screen = screens[index];
			NSRect rect = [screen frame];
			return rect.size.width;
		}

		int height(int index) {
			NSArray *screens = [NSScreen screens];
			NSScreen *screen = screens[index];
			NSRect rect = [screen frame];
			return rect.size.height;
		}

		int x(int index) {
			NSArray *screens = [NSScreen screens];
			NSScreen *screen = screens[index];
			NSRect rect = [screen frame];
			return rect.origin.x;
		}

		int y(int index) {
			NSArray *screens = [NSScreen screens];
			NSScreen *screen = screens[index];
			NSRect rect = [screen frame];
			return rect.origin.y;
		}

		bool isPrimary(int index) {
			NSArray *screens = [NSScreen screens];
			NSScreen *mainScreen = [NSScreen mainScreen];
			return mainScreen == screens[index];
		}
	}
}
