#include "pch.h"

#include <Kore/Threads/Event.h>

#include <Windows.h>

using namespace Kore;

void Event::create() {
	event = CreateEvent(0, 0, 0, 0);
}

void Event::destroy() {
	CloseHandle(event);
}

void Event::signal() {
	SetEvent(event);
}

void Event::wait() {
	WaitForSingleObject(event, INFINITE);
}

bool Event::tryToWait(double seconds) {
	return WaitForSingleObject(event, (DWORD)(seconds * 1000.0)) != WAIT_TIMEOUT;
}

void Event::reset() {
	ResetEvent(event);
}
