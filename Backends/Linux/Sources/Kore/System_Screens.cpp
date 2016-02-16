#include "pch.h"

#include "System_Screens.h"

#include <Kore/Log.h>

#include <cstdio>
#include <cstdlib>

namespace Kore { namespace System { namespace Monitor {
    void fatalError(const char* message) {
        printf("main: %s\n", message);
        exit(1);
    }

	enum { MAXIMUM_SCREEN_COUNT = 10 };

	KoreScreen screens[10];
	int screenCounter = -1;
	bool initialized = false;

    void enumDisplayMonitors( KoreScreen screens[], int & screenCounter );

	void enumerate() {
		if (initialized) {
			return;
		}

		enumDisplayMonitors(screens, screenCounter);
		initialized = true;
	}

	int count() {
		return screenCounter + 1;
	}

	const KoreScreen *
	primaryScreen() {
		for (int screenIndex = 0; screenIndex < MAXIMUM_SCREEN_COUNT; ++screenIndex) {
			const KoreScreen & screen = screens[screenIndex];

			if (screen.isAvailable && screen.isPrimary) {
				return &screen;
			}
		}

		log(Warning, "No primary screen defined");
		return nullptr;
	}

	const KoreScreen *
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
