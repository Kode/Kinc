#include "pch.h"

#include "System_Screens.h"

#include <Kore/Log.h>

#include <X11/extensions/Xrandr.h>

#include <cstdio>
#include <cstdlib>

/*
    TODO (DK) libXinerama version (someone said it's obsolete, use libXrandr)

#include <cstdlib>
#include <iostream>

#include <X11/extensions/Xinerama.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
	bool success=false;
	Display *d=XOpenDisplay(NULL);
	if (d) {
		int dummy1, dummy2;
		if (XineramaQueryExtension(d, &dummy1, &dummy2)) {
			if (XineramaIsActive(d)) {
				int heads=0;
				XineramaScreenInfo *p=XineramaQueryScreens(d, &heads);
				if (heads>0) {
					for (int x=0; x<heads; ++x)
						cout << "Head " << x+1 << " of " << heads << ": " <<
							p[x].width << "x" << p[x].height << " at " <<
							p[x].x_org << "," << p[x].y_org << endl;
					success=true;
				} else cout << "XineramaQueryScreens says there aren't any" << endl;
				XFree(p);
			} else cout << "Xinerama not active" << endl;
		} else cout << "No Xinerama extension" << endl;
		XCloseDisplay(d);
	} else cout << "Can't open display" << endl;

	return (success ? EXIT_SUCCESS : EXIT_FAILURE);
}
*/

namespace { namespace impl {
    Display * dpy;
}}

namespace {
    void fatalError(const char* message) {
        printf("main: %s\n", message);
        exit(1);
    }
}

namespace Kore { namespace System { namespace Monitor {
	enum { MAXIMUM_SCREEN_COUNT = 10 };

	Screen screens[10];
	int screenCounter = -1;
	bool initialized = false;

/*	bool queryInformation( HMONITOR monitor ) {
		MONITORINFOEXA info;
		memset(&info, 0, sizeof(MONITORINFOEXA));
		info.cbSize = sizeof(MONITORINFOEXA);

		if (GetMonitorInfoA(monitor, &info) == FALSE) {
			return false;
		}

		int freeSlot = 0;

		for (; freeSlot < MAXIMUM_SCREEN_COUNT; ++freeSlot) {
			if (screens[freeSlot].id == monitor) {
				return false;
			}

			if (screens[freeSlot].id == NULL) {
				break;
			}
		}

		Screen & screen = screens[freeSlot];

		strcpy_s(screen.name, 32, info.szDevice);
		screen.id = monitor;
		screen.isAvailable = true;
		screen.isPrimary = info.dwFlags == MONITORINFOF_PRIMARY;
		screen.x = info.rcMonitor.left;
		screen.y = info.rcMonitor.top;
		screen.width = info.rcMonitor.right - info.rcMonitor.left;
		screen.height = info.rcMonitor.bottom - info.rcMonitor.top;
		return true;
	}

	BOOL CALLBACK enumerationCallback( HMONITOR monitor, HDC, LPRECT, LPARAM lparam ) {
		bool isNew = queryInformation(monitor);

		if (isNew) {
			++screenCounter;
			return TRUE;
		} else {
			return FALSE;
		}
	}
*/

	void enumDisplayMonitors( Display * dpy ) {

	}

	void enumerate() {
		if (initialized) {
			return;
		}

        impl::dpy = XOpenDisplay(NULL);

        if (impl::dpy == NULL) {
            fatalError("could not open display");
        }

        int majorVersion;
        int minorVersion;

        Status status = XRRQueryVersion(impl::dpy, &majorVersion, &minorVersion);

		initialized = true;
		enumDisplayMonitors(impl::dpy);
	}

	int count() {
		return screenCounter + 1;
	}

	const Screen *
	primaryScreen() {
		for (int screenIndex = 0; screenIndex < MAXIMUM_SCREEN_COUNT; ++screenIndex) {
			const Screen & screen = screens[screenIndex];

			if (screen.isAvailable && screen.isPrimary) {
				return &screen;
			}
		}

		log(Warning, "No primary screen defined");
		return nullptr;
	}

	const Screen *
	screenById( int id ) {
        /*
		char displayId[32];
		sprintf_s(displayId, "\\\\.\\DISPLAY%i", id);

		for (int screenIndex = 0; screenIndex < MAXIMUM_SCREEN_COUNT; ++screenIndex) {
			const Screen & screen = screens[screenIndex];

			if (strcmp(screen.name, displayId) == 0) {
				return &screen;
			}
		}
        */
        log(Warning, "implement me");
		log(Warning, "No screen with id \"%i\" found", id);
		return nullptr;
	}
}}}
