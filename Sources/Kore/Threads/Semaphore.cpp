#include "pch.h"

#include "Semaphore.h"

using namespace Kore;

void Semaphore::create(int current, int max) {
	kinc_semaphore_init(&semaphore, current, max);
}

void Semaphore::destroy() {
	kinc_semaphore_destroy(&semaphore);
}

void Semaphore::release(int count) {
	kinc_semaphore_release(&semaphore, count);
}

void Semaphore::acquire() {
	kinc_semaphore_acquire(&semaphore);
}

bool Semaphore::tryToAcquire(double seconds) {
	return kinc_semaphore_try_to_acquire(&semaphore, seconds);
}
