#include "pch.h"

#include <kinc/threads/semaphore.h>

#include <assert.h>

void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max) {
	kinc_event_init(&semaphore->impl.event);
	kinc_mutex_init(&semaphore->impl.mutex);
	semaphore->impl.current = current;
	semaphore->impl.max = max;
}

void kinc_semaphore_destroy(kinc_semaphore_t *semaphore) {
	kinc_event_destroy(&semaphore->impl.event);
	kinc_mutex_destroy(&semaphore->impl.mutex);
}

void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count) {
	kinc_mutex_lock(&semaphore->impl.mutex);
	assert(semaphore->impl.current + count <= semaphore->impl.max);
	for (int i = 0; i < count; ++i) {
		++semaphore->impl.current;
		kinc_event_signal(&semaphore->impl.event);
	}
	kinc_mutex_unlock(&semaphore->impl.mutex);
}

void kinc_semaphore_acquire(kinc_semaphore_t *semaphore) {
	kinc_mutex_lock(&semaphore->impl.mutex);
	while (semaphore->impl.current <= 0) {
		kinc_event_wait(&semaphore->impl.event);
	}
	--semaphore->impl.current;
	kinc_mutex_unlock(&semaphore->impl.mutex);
}

bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double seconds) {
	kinc_mutex_lock(&semaphore->impl.mutex);
	if (semaphore->impl.current > 0) {
		--semaphore->impl.current;
		kinc_mutex_unlock(&semaphore->impl.mutex);
		return true;
	}
	kinc_mutex_unlock(&semaphore->impl.mutex);
	return false;
}
