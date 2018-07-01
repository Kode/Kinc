#pragma once

#include <vulkan/vulkan.h>

namespace Kore {
	namespace Graphics5 {
		class PipelineState;
	}
}

class CommandList5Impl {
public:
	Kore::Graphics5::PipelineState* _currentPipeline;
	int _indexCount;
	VkCommandBuffer _buffer;

protected:
	bool closed;
};
