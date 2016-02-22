#include "pch.h"

#include <Kore/Log.h>

#include "Display.h"

#include <cstdio>
#include <cstdlib>

namespace Kore { namespace Display {
    void fatalError(const char* message) {
        printf("main: %s\n", message);
        exit(1);
    }

	enum { MAXIMUM_DISPLAY_COUNT = 10 };

	DeviceInfo displays[MAXIMUM_DISPLAY_COUNT];
	int displayCounter = -1;
	bool initialized = false;

    void enumDisplayMonitors( DeviceInfo screens[], int & displayCounter );

	void enumerate() {
		if (initialized) {
			return;
		}

		initialized = true;
		enumDisplayMonitors(displays, displayCounter);
	}

	int count() {
		return displayCounter + 1;
	}

    int width( int index ) {
        return displays[index].width;
    }

    int height( int index ) {
        return displays[index].height;
    }

	const DeviceInfo *
	primaryScreen() {
		for (int index = 0; index < MAXIMUM_DISPLAY_COUNT; ++index) {
			const DeviceInfo & info = displays[index];

			if (info.isAvailable && info.isPrimary) {
				return &info;
			}
		}

        if (!displays[0].isAvailable) {
            log(Warning, "No display attached?");
            // TODO (DK) throw exception?
            return nullptr;
        }

		log(Warning, "No primary display defined, returning first display");
		return &displays[0];
	}

	const DeviceInfo *
	screenById( int id ) {
		for (int index = 0; index < MAXIMUM_DISPLAY_COUNT; ++index) {
			const DeviceInfo & info = displays[index];

			if (info.number == id) {
				return &info;
			}
		}

        if (!displays[0].isAvailable) {
            log(Warning, "No display available");
            // TODO (DK) throw exception?
            return nullptr;
        }

		log(Warning, "No display with id \"%i\" found, returning first display", id);
		return &displays[0];
	}
}}
