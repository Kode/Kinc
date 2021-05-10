#pragma once

#include <kinc/global.h>

#include <kinc/backend/mutex.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_mutex_impl_t impl;
} kinc_mutex_t;

KINC_FUNC void kinc_mutex_init(kinc_mutex_t *mutex);
KINC_FUNC void kinc_mutex_destroy(kinc_mutex_t *mutex);
KINC_FUNC void kinc_mutex_lock(kinc_mutex_t *mutex);
KINC_FUNC bool kinc_mutex_try_to_lock(kinc_mutex_t *mutex);
KINC_FUNC void kinc_mutex_unlock(kinc_mutex_t *mutex);

typedef struct {
	kinc_uber_mutex_impl_t impl;
} kinc_uber_mutex_t;

KINC_FUNC bool kinc_uber_mutex_init(kinc_uber_mutex_t *mutex, const char *name);
KINC_FUNC void kinc_uber_mutex_destroy(kinc_uber_mutex_t *mutex);
KINC_FUNC void kinc_uber_mutex_lock(kinc_uber_mutex_t *mutex);
KINC_FUNC void kinc_uber_mutex_unlock(kinc_uber_mutex_t *mutex);

#ifdef __cplusplus
}
#endif
