#pragma once

#include <vulkan/vulkan.h>

typedef struct {
	//Kore::Graphics5::PipelineState* _currentPipeline;
	int _indexCount;
	VkCommandBuffer _buffer;
	bool closed;
} CommandList5Impl;
