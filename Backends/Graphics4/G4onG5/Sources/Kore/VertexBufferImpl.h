#pragma once

#include <Kinc/Graphics5/VertexBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int myCount;
	//void prepareLock();
	kinc_g5_vertex_buffer_t _buffer;
	int _currentIndex;
	const int _multiple;
	uint64_t _lastFrameNumber;
} VertexBufferImpl;

#ifdef __cplusplus
}
#endif
