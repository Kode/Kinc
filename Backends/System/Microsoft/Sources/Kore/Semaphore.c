#include "pch.h"

#include <Kinc/Threads/Semaphore.h>

#include <assert.h>


void Kinc_Semaphore_Create(Kinc_Semaphore *semaphore, int current, int max) {
	Kinc_Event_Create(&semaphore->impl.event);
	Kinc_Mutex_Create(&semaphore->impl.mutex);
	semaphore->impl.current = current;
	semaphore->impl.max = max;
}

void Kinc_Semaphore_Destroy(Kinc_Semaphore *semaphore) {
	Kinc_Event_Destroy(&semaphore->impl.event);
	Kinc_Mutex_Destroy(&semaphore->impl.mutex);
}

void Kinc_Semaphore_Release(Kinc_Semaphore *semaphore, int count) {
	Kinc_Mutex_Lock(&semaphore->impl.mutex);
	assert(semaphore->impl.current + count <= semaphore->impl.max);
	for (int i = 0; i < count; ++i) {
		++semaphore->impl.current;
		Kinc_Event_Signal(&semaphore->impl.event);
	}
	Kinc_Mutex_Unlock(&semaphore->impl.mutex);
}

void Kinc_Semaphore_Acquire(Kinc_Semaphore *semaphore) {
	Kinc_Mutex_Lock(&semaphore->impl.mutex);
	while (semaphore->impl.current <= 0) {
		Kinc_Event_Wait(&semaphore->impl.event);
	}
	--semaphore->impl.current;
	Kinc_Mutex_Unlock(&semaphore->impl.mutex);
}

bool Kinc_Semaphore_TryToAcquire(Kinc_Semaphore *semaphore, double seconds) {
	Kinc_Mutex_Lock(&semaphore->impl.mutex);
	if (semaphore->impl.current > 0) {
		--semaphore->impl.current;
		Kinc_Mutex_Unlock(&semaphore->impl.mutex);
		return true;
	}
	Kinc_Mutex_Unlock(&semaphore->impl.mutex);
	return false;
}
