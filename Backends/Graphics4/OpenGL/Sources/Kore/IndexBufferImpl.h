#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#if defined(KORE_ANDROID) || defined(KORE_PI)
	uint16_t *shortData;
#endif
	int *data;
	int myCount;
	unsigned bufferId;
} kinc_g4_index_buffer_impl_t;

#ifdef __cplusplus
}
#endif
