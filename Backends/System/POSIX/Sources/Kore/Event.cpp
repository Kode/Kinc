#include "pch.h"

#include <kinc/threads/event.h>

#include <assert.h>
#include <errno.h>
#include <sys/time.h>

void kinc_event_init(kinc_event_t *event) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&event->impl.mutex, &attr);
	pthread_cond_init(&event->impl.condvar, NULL);
}

void kinc_event_destroy(kinc_event_t *event) {
	pthread_cond_destroy(&event->impl.condvar);
	pthread_mutex_destroy(&event->impl.mutex);
}

void kinc_event_signal(kinc_event_t *event) {
	pthread_mutex_lock(&event->impl.mutex);
	pthread_cond_signal(&event->impl.condvar);
	pthread_mutex_unlock(&event->impl.mutex);
}

void kinc_event_wait(kinc_event_t *event) {
	pthread_mutex_lock(&event->impl.mutex);
	pthread_cond_wait(&event->impl.condvar, &event->impl.mutex);
	pthread_mutex_unlock(&event->impl.mutex);
}

bool kinc_event_try_to_wait(kinc_event_t *event, double seconds) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	
	int isec = (int)seconds;
	int usec = (int)((seconds - isec) * 1000000.0);
	timespec spec;
	spec.tv_nsec = (tv.tv_usec + usec) * 1000;
	if (spec.tv_nsec > 1000000000) {
		spec.tv_nsec -= 1000000000;
		isec += 1;
	}
	spec.tv_sec = tv.tv_sec + isec;
	
	pthread_mutex_lock(&event->impl.mutex);
	int result = pthread_cond_timedwait(&event->impl.condvar, &event->impl.mutex, &spec);
	assert(result == ETIMEDOUT || result == 0);
	pthread_mutex_unlock(&event->impl.mutex);
	return result == 0;
}

void kinc_event_reset(kinc_event_t *event) {
	pthread_cond_destroy(&event->impl.condvar);
	pthread_cond_init(&event->impl.condvar, NULL);
}
