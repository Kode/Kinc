#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Log.h>
#include <Kore/Windows.h>

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#undef RegisterClass

using namespace Kore;

namespace {
	const int maximumDisplays = 10;
	Display displays[maximumDisplays];
	DEVMODEA originalModes[maximumDisplays];
	int screenCounter = 0;

	enum MONITOR_DPI_TYPE { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI };
	typedef HRESULT(WINAPI* GetDpiForMonitorType)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
	GetDpiForMonitorType MyGetDpiForMonitor = nullptr;

	BOOL CALLBACK enumerationCallback(HMONITOR monitor, HDC, LPRECT, LPARAM lparam) {
		MONITORINFOEXA info;
		memset(&info, 0, sizeof(MONITORINFOEXA));
		info.cbSize = sizeof(MONITORINFOEXA);

		if (GetMonitorInfoA(monitor, &info) == FALSE) {
			return FALSE;
		}

		int freeSlot = 0;
		for (; freeSlot < maximumDisplays; ++freeSlot) {
			if (displays[freeSlot]._data.id == monitor) {
				return FALSE;
			}

			if (displays[freeSlot]._data.id == NULL) {
				break;
			}
		}

		Display& display = displays[freeSlot];
		strcpy_s(display._data.name, 32, info.szDevice);
		display._data.index = freeSlot;
		display._data.id = monitor;
		display._data.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
		display._data.available = true;
		display._data.x = info.rcMonitor.left;
		display._data.y = info.rcMonitor.top;
		display._data.width = info.rcMonitor.right - info.rcMonitor.left;
		display._data.height = info.rcMonitor.bottom - info.rcMonitor.top;
	
		HDC hdc = CreateDCA(nullptr, display._data.name, nullptr, nullptr);
		display._data.ppi = GetDeviceCaps(hdc, LOGPIXELSX);
		int scale = GetDeviceCaps(hdc, SCALINGFACTORX);
		DeleteDC(hdc);

		unsigned dpiX, dpiY;
		if (MyGetDpiForMonitor != nullptr) {
			MyGetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
			display._data.ppi = (int)dpiX;
		}

		originalModes[freeSlot] = {0};
		originalModes[freeSlot].dmSize = sizeof(DEVMODEA);
		EnumDisplaySettingsA(display._data.name, ENUM_CURRENT_SETTINGS, &originalModes[freeSlot]);
		display._data.frequency = originalModes[freeSlot].dmDisplayFrequency;

		++screenCounter;
		return TRUE;
	}
}

void Windows::initDisplays() {
	HMODULE shcore = LoadLibraryA("Shcore.dll");
	if (shcore != nullptr) {
		MyGetDpiForMonitor = (GetDpiForMonitorType)GetProcAddress(shcore, "GetDpiForMonitor");
	}
	EnumDisplayMonitors(NULL, NULL, enumerationCallback, NULL);
}

Display* Windows::getDisplayForMonitor(HMONITOR monitor) {
	for (int i = 0; i < 10; ++i) {
		if (displays[i]._data.id == monitor) {
			return &displays[i];
		}
	}
	return nullptr;
}

int Display::count() {
	return screenCounter;
}

Display* Display::primary() {
	for (int screenIndex = 0; screenIndex < maximumDisplays; ++screenIndex) {
		Display& display = displays[screenIndex];

		if (display._data.available && display._data.primary) {
			return &display;
		}
	}

	log(Warning, "No primary display defined");
	return nullptr;
}

Display* Display::get(int index) {
	return &displays[index];
}

DisplayMode Display::availableMode(int index) {
	DEVMODEA devMode = {0};
	devMode.dmSize = sizeof(DEVMODEA);
	EnumDisplaySettingsA(_data.name, index, &devMode);
	DisplayMode mode;
	mode.width = devMode.dmPelsWidth;
	mode.height = devMode.dmPelsHeight;
	mode.frequency = devMode.dmDisplayFrequency;
	mode.bitsPerPixel = devMode.dmBitsPerPel;
	return mode;
}

int Display::countAvailableModes() {
	DEVMODEA devMode = {0};
	devMode.dmSize = sizeof(DEVMODEA);
	int i = 0;
	for (; EnumDisplaySettingsA(_data.name, i, &devMode) != FALSE; ++i) { }
	return i;
}

bool setDisplayMode(Display* display, int width, int height, int bpp, int frequency) {
	display->_data.modeChanged = true;
	DEVMODEA mode = {0};
	mode.dmSize = sizeof(mode);
	strcpy((char*)mode.dmDeviceName, display->_data.name);
	mode.dmPelsWidth = width;
	mode.dmPelsHeight = height;
	mode.dmBitsPerPel = bpp;
	mode.dmDisplayFrequency = frequency;
	mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	
	return ChangeDisplaySettingsA(&mode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
}

void Windows::restoreDisplay(int display) {
	if (displays[display]._data.modeChanged) {
		ChangeDisplaySettingsA(&originalModes[display], 0);
	}
}

void Windows::restoreDisplays() {
	for (int i = 0; i < maximumDisplays; ++i) {
		restoreDisplay(i);	
	}
}

int Display::pixelsPerInch() {
	return _data.ppi;
}

DisplayData::DisplayData() : ppi(96), modeChanged(false) {
	name[0] = 0;
}

bool Display::available() {
	return _data.available;
}

const char* Display::name() {
	return _data.name;
}

int Display::x() {
	return _data.x;
}

int Display::y() {
	return _data.y;
}

int Display::width() {
	return _data.width;
}

int Display::height() {
	return _data.height;
}

int Display::frequency() {
	return _data.frequency;
}

Display::Display() {

}
