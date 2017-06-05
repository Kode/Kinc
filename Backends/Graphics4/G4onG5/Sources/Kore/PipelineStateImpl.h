#pragma once

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics5/Shader.h>
#include <Kore/Graphics5/PipelineState.h>

namespace Kore {
	namespace Graphics4 {
		class Shader;
	}

	class PipelineStateImpl {
	public:
		PipelineStateImpl();

		Graphics5::PipelineState* _pipeline;
	};

	class ConstantLocationImpl {
	public:
		Graphics5::ConstantLocation _location;
	};

	class AttributeLocationImpl {};
}
