#pragma once

#include <kinc/graphics5/vertexstructure.h>

#include <objc/runtime.h>

typedef struct {
	//void unset();
	int myCount;
	int myStride;
	id mtlBuffer;
	bool gpuMemory;
	int lastStart;
	int lastCount;
	//static Graphics5::VertexBuffer* current;
} VertexBuffer5Impl;
