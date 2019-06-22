#pragma once

#include <Kore/SemaphoreImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_semaphore_impl_t impl;
} kinc_semaphore_t;

void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max);
void kinc_semaphore_destroy(kinc_semaphore_t *semaphore);
void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count);
void kinc_semaphore_acquire(kinc_semaphore_t *semaphore);
bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double seconds);

#ifdef __cplusplus
}
#endif
