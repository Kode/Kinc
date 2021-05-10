#include <Kore/Threads/Event.h>

using namespace Kore;

void Event::create() {}

void Event::destroy() {}

void Event::signal() {}

void Event::wait() {}

bool Event::tryToWait(double seconds) {
	return true;
}

void Event::reset() {}
