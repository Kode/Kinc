#include "pch.h"

#include "Display.h"

#include <Kore/Log.h>

#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#undef RegisterClass

// TODO (DK) use the windows defines versions instead of ascii functionA (GetMonitorInfoA -> GetMonitorInfo)?
namespace Kore {
	namespace Display {
		enum { MAXIMUM_DISPLAY_COUNT = 10 };

		DeviceInfo displays[10];
		int screenCounter = -1;
		bool initialized = false;

		volatile static struct StaticInitializer {
			StaticInitializer() {
				enumerate();
			}
		} displayInitializer;

		bool queryInformation(HMONITOR monitor) {
			MONITORINFOEXA info;
			memset(&info, 0, sizeof(MONITORINFOEXA));
			info.cbSize = sizeof(MONITORINFOEXA);

			if (GetMonitorInfoA(monitor, &info) == FALSE) {
				return false;
			}

			int freeSlot = 0;

			for (; freeSlot < MAXIMUM_DISPLAY_COUNT; ++freeSlot) {
				if (displays[freeSlot].id == monitor) {
					return false;
				}

				if (displays[freeSlot].id == NULL) {
					break;
				}
			}

			DeviceInfo& di = displays[freeSlot];

			strcpy_s(di.name, 32, info.szDevice);
			di.id = monitor;
			di.isAvailable = true;
			di.isPrimary = info.dwFlags == MONITORINFOF_PRIMARY;
			di.x = info.rcMonitor.left;
			di.y = info.rcMonitor.top;
			di.width = info.rcMonitor.right - info.rcMonitor.left;
			di.height = info.rcMonitor.bottom - info.rcMonitor.top;
			return true;
		}

		BOOL CALLBACK enumerationCallback(HMONITOR monitor, HDC, LPRECT, LPARAM lparam) {
			bool isNew = queryInformation(monitor);

			if (isNew) {
				++screenCounter;
				return TRUE;
			}
			else {
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

		const DeviceInfo* primary() {
			for (int screenIndex = 0; screenIndex < MAXIMUM_DISPLAY_COUNT; ++screenIndex) {
				const DeviceInfo& info = displays[screenIndex];

				if (info.isAvailable && info.isPrimary) {
					return &info;
				}
			}

			log(Warning, "No primary display defined");
			return nullptr;
		}

		const DeviceInfo* byId(int id) {
			// TODO (DK) find a better way to identify than strcmp
			char displayId[32];
			sprintf_s(displayId, "\\\\.\\DISPLAY%i", id);

			DeviceInfo* last = nullptr;
			for (int screenIndex = 0; screenIndex < MAXIMUM_DISPLAY_COUNT; ++screenIndex) {
				last = &displays[screenIndex];

				if (strcmp(last->name, displayId) == 0) {
					return last;
				}
			}

			if (id != 0 || last == nullptr) {
				log(Warning, "No display with id \"%i\" found", id);
			}
			return last;
		}

		int width(int index) {
			return displays[index].width;
		}

		int height(int index) {
			return displays[index].height;
		}

		int x(int index) {
			return displays[index].x;
		}

		int y(int index) {
			return displays[index].y;
		}

		bool isPrimary(int index) {
			return displays[index].isPrimary;
		}
	}
}
