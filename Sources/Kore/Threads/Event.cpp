#include "pch.h"

#include "Event.h"

using namespace Kore;

void Event::create(bool autoClear) {
	kinc_event_init(&event, autoClear);
}

void Event::destroy() {
	kinc_event_destroy(&event);
}

void Event::signal() {
	kinc_event_signal(&event);
}

void Event::wait() {
	kinc_event_wait(&event);
}

bool Event::tryToWait(double seconds) {
	return kinc_event_try_to_wait(&event, seconds);
}

void Event::reset() {
	kinc_event_reset(&event);
}
