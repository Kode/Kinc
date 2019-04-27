#pragma once

#include <Kinc/Graphics4/VertexBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_g4_vertex_buffer_t buffer;
	int myCount;
	int myStart;
} VertexBuffer5Impl;

#ifdef __cplusplus
}
#endif
