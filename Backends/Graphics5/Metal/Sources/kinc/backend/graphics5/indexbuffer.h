#pragma once

typedef struct {
	//void unset();
	void *mtlBuffer;
	int myCount;
	bool gpuMemory;
	//static Graphics5::IndexBuffer* current;
} IndexBuffer5Impl;
