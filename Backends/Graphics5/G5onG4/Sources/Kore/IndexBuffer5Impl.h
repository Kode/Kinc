#pragma once

#include <Kinc/Graphics4/IndexBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_g4_index_buffer_t buffer;
	int myCount;
} IndexBuffer5Impl;

#ifdef __cplusplus
}
#endif
