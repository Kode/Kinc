#pragma once

#include <Kinc/Threads/Event.h>
#include <Kinc/Threads/Mutex.h>

typedef struct {
	Kinc_Event event;
	Kinc_Mutex mutex;
	int current;
	int max;
} Kinc_SemaphoreImpl;
