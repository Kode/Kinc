#include "pch.h"

#include <kinc/threads/threadlocal.h>

void kinc_thread_local_init(kinc_thread_local_t *local) {}

void kinc_thread_local_destroy(kinc_thread_local_t *local) {}

void* kinc_thread_local_get(kinc_thread_local_t *local) {
	return nullptr;
}

void kinc_thread_local_set(kinc_thread_local_t *local, void* data) {}
