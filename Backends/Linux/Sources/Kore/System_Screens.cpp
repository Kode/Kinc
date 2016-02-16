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

        if (!screens[0].isAvailable) {
            log(Warning, "No screen defined");
            // TODO (DK) throw exception?
            return nullptr;
        }

		log(Warning, "No primary screen defined, returning first screen");
		return &screens[0];
	}

	const KoreScreen *
	screenById( int id ) {
		for (int screenIndex = 0; screenIndex < MAXIMUM_SCREEN_COUNT; ++screenIndex) {
			const KoreScreen & screen = screens[screenIndex];

			if (screen.number == id) {
				return &screen;
			}
		}

        if (!screens[0].isAvailable) {
            log(Warning, "No screen available");
            // TODO (DK) throw exception?
            return nullptr;
        }

		log(Warning, "No screen with id \"%i\" found, returning first screen", id);
		return &screens[0];
	}
}}}
