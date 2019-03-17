#include "pch.h"

#include "Semaphore.h"

using namespace Kore;

void Semaphore::create(int current, int max) {
	Kinc_Semaphore_Create(&semaphore, current, max);
}

void Semaphore::destroy() {
	Kinc_Semaphore_Destroy(&semaphore);
}

void Semaphore::release(int count) {
	Kinc_Semaphore_Release(&semaphore, count);
}

void Semaphore::acquire() {
	Kinc_Semaphore_Acquire(&semaphore);
}

bool Semaphore::tryToAcquire(double seconds) {
	return Kinc_Semaphore_TryToAcquire(&semaphore, seconds);
}
