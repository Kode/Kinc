#pragma once

#include <kinc/backend/threadlocal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_thread_local_impl_t impl;
} kinc_thread_local_t;

KINC_FUNC void kinc_thread_local_init(kinc_thread_local_t *local);
KINC_FUNC void kinc_thread_local_destroy(kinc_thread_local_t *local);
KINC_FUNC void *kinc_thread_local_get(kinc_thread_local_t *local);
KINC_FUNC void kinc_thread_local_set(kinc_thread_local_t *local, void *data);

#ifdef __cplusplus
}
#endif
