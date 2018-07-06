#include "pch.h"

#include <Kore/Display.h>
#include <Kore/Log.h>

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#undef RegisterClass

using namespace Kore;

namespace {
	const int maximumDisplays = 10;
	Display displays[maximumDisplays];
	int screenCounter = -1;
}

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
	display._data.id = monitor;
	display._data.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
	display._data.available = true;
	display._data.x = info.rcMonitor.left;
	display._data.y = info.rcMonitor.top;
	display._data.width = info.rcMonitor.right - info.rcMonitor.left;
	display._data.height = info.rcMonitor.bottom - info.rcMonitor.top;
	
	HDC hdc = CreateDCA(nullptr, display._data.name, nullptr, nullptr);
	display._data.ppi = GetDeviceCaps(hdc, LOGPIXELSX);
	DeleteDC(hdc);

	DEVMODEA devMode = {0};
	devMode.dmSize = sizeof(DEVMODEA);
	EnumDisplaySettingsA(display._data.name, ENUM_CURRENT_SETTINGS, &devMode);
	display._data.frequency = devMode.dmDisplayFrequency;

	++screenCounter;
	return TRUE;
}

void initDisplays() {
	//SetProcessDPIAware(); // TODO: Use manifest value instead and test
	EnumDisplayMonitors(NULL, NULL, enumerationCallback, NULL);
}

int Display::count() {
	return screenCounter + 1;
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
	return mode;
}

int Display::countAvailableModes() {
	DEVMODEA devMode = {0};
	devMode.dmSize = sizeof(DEVMODEA);
	int i = 0;
	for (; EnumDisplaySettingsA(_data.name, i, &devMode) != FALSE; ++i) { }
	return i;
}

/*void Display::setMode(int index) {
#pragma message("TODO (DK) implement changeResolution(w,h,fs) for d3d")

#if !defined(KORE_OPENGL) && !defined(KORE_VULKAN)
Application::the()->setWidth(width);
Application::the()->setHeight(height);
Application::the()->setFullscreen(fullscreen);

if (!Application::the()->fullscreen()) {
    uint yres = GetSystemMetrics(SM_CYSCREEN);

    // Fenster rechts von Textkonsole positionieren
    RECT r;
    r.left   = 8 * 80 + 44;
    r.top    = 0;
    r.right  = r.left + Application::the()->width() - 1;
    r.bottom = r.top + Application::the()->height() - 1;
    uint h = r.bottom - r.top + 1;
    DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&r, dwStyle, FALSE); // Rahmen usw. miteinberechnen
    MoveWindow(hwnd, r.left, (yres - h) >> 1, r.right - r.left + 1, r.bottom - r.top + 1, TRUE);

    Graphics::changeResolution(width, height);
}
#endif
}*/

/*int Kore::System::desktopWidth() {
	RECT size;
	const HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &size);
	return size.right;
}

int Kore::System::desktopHeight() {
	RECT size;
	const HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &size);
	return size.bottom;
}*/

int Display::pixelsPerInch() {
	return _data.ppi;
}

DisplayData::DisplayData() : ppi(72) {
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
