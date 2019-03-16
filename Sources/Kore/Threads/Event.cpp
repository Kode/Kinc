#include "pch.h"

#include "Event.h"

using namespace Kore;

void Event::create() {
	Kinc_Event_Create(&event);
}

void Event::destroy() {
	Kinc_Event_Destroy(&event);
}

void Event::signal() {
	Kinc_Event_Signal(&event);
}

void Event::wait() {
	Kinc_Event_Wait(&event);
}

bool Event::tryToWait(double seconds) {
	return Kinc_Event_TryToWait(&event, seconds);
}

void Event::reset() {
	Kinc_Event_Reset(&event);
}
