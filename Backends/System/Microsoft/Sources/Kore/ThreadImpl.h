#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *handle;
	void *param;
	void (*func)(void *param);
} kinc_thread_impl_t;

void kinc_microsoft_threads_init();
void kinc_microsoft_threads_quit();

#ifdef __cplusplus
}
#endif
