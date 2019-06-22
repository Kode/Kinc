#pragma once

#include <objc/runtime.h>

typedef struct {
	//void unset();
	id mtlBuffer;
	int myCount;
	bool gpuMemory;
	//static Graphics5::IndexBuffer* current;
} IndexBuffer5Impl;
