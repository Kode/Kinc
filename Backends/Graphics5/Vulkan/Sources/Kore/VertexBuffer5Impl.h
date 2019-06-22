#pragma once

#include <kinc/graphics5/vertexstructure.h>

#include <vulkan/vulkan.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef RegisterClass
#undef RegisterClass
#endif

struct Vertices {
	VkBuffer buf;
	VkDeviceMemory mem;
};

typedef struct {
//void unset();
	float* data;
	int myCount;
	int myStride;
	unsigned bufferId;
	kinc_g5_vertex_structure_t structure;
	VkMemoryAllocateInfo mem_alloc;
	int instanceDataStepRate;
	//int setVertexAttributes(int offset);
	struct Vertices vertices;
	//static Graphics5::VertexBuffer* current;
} VertexBuffer5Impl;
