#pragma once

#include <kinc/backend/semaphore.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_semaphore_impl_t impl;
} kinc_semaphore_t;

KINC_FUNC void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max);
KINC_FUNC void kinc_semaphore_destroy(kinc_semaphore_t *semaphore);
KINC_FUNC void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count);
KINC_FUNC void kinc_semaphore_acquire(kinc_semaphore_t *semaphore);
KINC_FUNC bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double seconds);

#ifdef __cplusplus
}
#endif
