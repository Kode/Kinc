#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics6/buffer.h>

/*! \file buffer.h
    \brief Provides functions for setting up and using buffers.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g6_buffer_usage_bits {
	KINC_G6_BUFFER_USAGE_COPY_SRC = 0x001,
	KINC_G6_BUFFER_USAGE_COPY_DST = 0x002,
	KINC_G6_BUFFER_USAGE_VERTEX = 0x004,
	KINC_G6_BUFFER_USAGE_INDEX = 0x008,
	KINC_G6_BUFFER_USAGE_UNIFORM = 0x010,
	KINC_G6_BUFFER_USAGE_STORAGE = 0x020
} kinc_g6_buffer_usage_bits;

typedef uint32_t kinc_g6_buffer_usage_t;

typedef struct kinc_g6_buffer {
	kinc_g6_buffer_impl_t impl;
} kinc_g6_buffer_t;

typedef struct kinc_g6_buffer_descriptor {
	uint64_t size;
	kinc_g6_buffer_usage_t usage;
	bool gpuMemory;
} kinc_g6_buffer_descriptor_t;

KINC_FUNC void kinc_g6_buffer_init(kinc_g6_buffer_t *buffer, const kinc_g6_buffer_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_buffer_destroy(kinc_g6_buffer_t *buffer);
KINC_FUNC void *kinc_g6_buffer_map(kinc_g6_buffer_t *buffer, int start, int range);
KINC_FUNC void kinc_g6_buffer_unmap(kinc_g6_buffer_t *buffer);

#ifdef __cplusplus
}
#endif