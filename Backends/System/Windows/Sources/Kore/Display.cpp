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
	class WindowsDisplay : public Display {
	public:
		HMONITOR id;
		bool isPrimary;
		WindowsDisplay() : id(nullptr), isPrimary(false) {}
	};

	const int maximumDisplays = 10;
	WindowsDisplay displays[maximumDisplays];
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
		if (displays[freeSlot].id == monitor) {
			return FALSE;
		}

		if (displays[freeSlot].id == NULL) {
			break;
		}
	}

	WindowsDisplay& display = displays[freeSlot];
	strcpy_s(display.name, 32, info.szDevice);
	display.id = monitor;
	display.isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
	display.available = true;
	display.x = info.rcMonitor.left;
	display.y = info.rcMonitor.top;
	display.width = info.rcMonitor.right - info.rcMonitor.left;
	display.height = info.rcMonitor.bottom - info.rcMonitor.top;


	HDC hdc = CreateDCA(nullptr, display.name, nullptr, nullptr);
	display.pixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSX);
	DeleteDC(hdc);

	DEVMODEA devMode = {0};
	devMode.dmSize = sizeof(DEVMODEA);
	EnumDisplaySettingsA(display.name, ENUM_CURRENT_SETTINGS, &devMode);
	display.frequency = devMode.dmDisplayFrequency;

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
		WindowsDisplay& display = displays[screenIndex];

		if (display.available && display.isPrimary) {
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
	EnumDisplaySettingsA(name, index, &devMode);
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
	for (; EnumDisplaySettingsA(name, i, &devMode) != FALSE; ++i) { }
	return i;
}

void Display::setMode(int index) {
#pragma message("TODO (DK) implement changeResolution(w,h,fs) for d3d")

#if !defined(KORE_OPENGL) && !defined(KORE_VULKAN)
/*Application::the()->setWidth(width);
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
*/
#endif
}

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
