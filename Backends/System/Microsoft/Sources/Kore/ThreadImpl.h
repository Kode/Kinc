#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *handle;
	void *param;
	void (*func)(void *param);
} Kinc_ThreadImpl;

#ifdef __cplusplus
}
#endif
