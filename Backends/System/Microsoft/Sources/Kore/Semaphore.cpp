#include "pch.h"

#include <Kore/Threads/Semaphore.h>

#include <assert.h>

using namespace Kore;

void Semaphore::create(int current, int max) {
	event.create();
	mutex.create();
	this->current = current;
	this->max = max;
}

void Semaphore::destroy() {
	event.destroy();
	mutex.destroy();
}

void Semaphore::release(int count) {
	mutex.lock();
	assert(current + count <= max);
	for (int i = 0; i < count; ++i) {
		++current;
		event.signal();
	}
	mutex.unlock();
}

void Semaphore::acquire() {
	mutex.lock();
	while (current <= 0) {
		event.wait();
	}
	--current;
	mutex.unlock();
}

bool Semaphore::tryToAcquire(double seconds) {
	mutex.lock();
	if (current > 0) {
		--current;
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}
