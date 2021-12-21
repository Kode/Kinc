#pragma once

typedef struct {
	void *metal_buffer;
	int count;
	bool gpu_memory;
	int format;
} IndexBuffer5Impl;
