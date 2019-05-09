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
	int _multiple;
	uint64_t _lastFrameNumber;
} Kinc_G4_VertexBufferImpl;

#ifdef __cplusplus
}
#endif
