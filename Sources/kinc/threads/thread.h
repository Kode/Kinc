#pragma once

#include <Kore/ThreadImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_thread_impl_t impl;
} kinc_thread_t;

KINC_FUNC void kinc_threads_init();
KINC_FUNC void kinc_threads_quit();

KINC_FUNC void kinc_thread_init(kinc_thread_t *thread, void (*func)(void *param), void *param);
KINC_FUNC void kinc_thread_wait_and_destroy(kinc_thread_t *thread);
KINC_FUNC bool kinc_thread_try_to_destroy(kinc_thread_t *thread);
KINC_FUNC void kinc_thread_sleep(int milliseconds);

#ifdef __cplusplus
}
#endif
