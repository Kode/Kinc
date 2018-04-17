#include "pch.h"

#include <Kore/Threads/Semaphore.h>

using namespace Kore;

void Semaphore::create(int current, int max) {}

void Semaphore::destroy() {}

void Semaphore::release(int count) {}

void Semaphore::acquire() {}

bool Semaphore::tryToAcquire(double seconds) {
	return true;
}
