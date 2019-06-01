#include "pch.h"

#include <kinc/threads/semaphore.h>

void Kinc_Semaphore_Create(Kinc_Semaphore *semaphore, int current, int max) {}

void Kinc_Semaphore_Destroy(Kinc_Semaphore *semaphore) {}

void Kinc_Semaphore_Release(Kinc_Semaphore *semaphore, int count) {}

void Kinc_Semaphore_Acquire(Kinc_Semaphore *semaphore) {}

bool Kinc_Semaphore_TryToAcquire(Kinc_Semaphore *semaphore, double seconds) {
	return true;
}
