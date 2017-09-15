#pragma once

namespace Kore {
	namespace Graphics5 {
		class PipelineState;
	}
}

class CommandList5Impl {
public:
	Kore::Graphics5::PipelineState* _currentPipeline;
	int _indexCount;
	Kore::s64 commands[1024];
	int commandIndex;
protected:
	bool closed;
};
