#include "pch.h"

#include "System_Screens.h"

#include <Kore/Log.h>

#include <cstdio>

// TODO (DK) use the windows defines version instead of ascii functionA (GetMonitorInfoA -> GetMonitorInfo)?
namespace Kore { namespace System { namespace Monitor {
	enum { MAXIMUM_SCREEN_COUNT = 10 };

	Screen screens[10];
	int screenCounter = -1;
	bool initialized = false;

	bool queryInformation( HMONITOR monitor ) {
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

	void enumerate() {
		if (initialized) {
			return;
		}

		initialized = true;
		EnumDisplayMonitors(NULL, NULL, enumerationCallback, NULL);
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
		//if (id < 0 || id > MAXIMUM_SCREEN_COUNT) {
		//	log(Error, "Invalid monitor id \"%i\"", id);
		//	return nullptr;
		//}

		char displayId[32];
		sprintf_s(displayId, "\\\\.\\DISPLAY%i", id);

		for (int screenIndex = 0; screenIndex < MAXIMUM_SCREEN_COUNT; ++screenIndex) {
			const Screen & screen = screens[screenIndex];

			if (strcmp(screen.name, displayId) == 0) {
				return &screen;
			}
		}

		log(Warning, "No screen with id \"%i\" found", id);
		return nullptr;
	}
}}}
