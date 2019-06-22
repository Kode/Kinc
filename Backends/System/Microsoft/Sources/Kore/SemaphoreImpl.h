#pragma once

#include <kinc/threads/event.h>
#include <kinc/threads/mutex.h>

typedef struct {
	kinc_event_t event;
	kinc_mutex_t mutex;
	int current;
	int max;
} kinc_semaphore_impl_t;
