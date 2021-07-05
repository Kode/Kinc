#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t *shortData;
	int *data;
	int myCount;
	unsigned usage;
	unsigned bufferId;
} kinc_g4_index_buffer_impl_t;

#ifdef __cplusplus
}
#endif
