#include "pch.h"

#include <kinc/threads/semaphore.h>
#include <kinc/system.h>

void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max) {
	sem_init(&semaphore->impl.semaphore, 0, current);
}

void kinc_semaphore_destroy(kinc_semaphore_t *semaphore) {
	sem_destroy(&semaphore->impl.semaphore);
}

void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count) {
	for (int i = 0; i < count; ++i) {
		sem_post(&semaphore->impl.semaphore);
	}
}

void kinc_semaphore_acquire(kinc_semaphore_t *semaphore) {
	sem_wait(&semaphore->impl.semaphore);
}

bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double seconds) {
	double now = kinc_time();
	do {
		if (sem_trywait(&semaphore->impl.semaphore) == 0) {
			return true;
		}
	}
	while (kinc_time() < now + seconds);
	return false;
}
