#pragma once

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics5/Shader.h>
#include <Kore/Graphics5/PipelineState.h>

namespace Kore {
	namespace Graphics4 {
		class Shader;
	}

	class ProgramImpl {
	public:
		ProgramImpl();

		Graphics5::Program _program;
		//Graphics5::PipelineState* _state;
	};

	class ConstantLocationImpl {
	public:
		Graphics5::ConstantLocation _location;
	};

	class AttributeLocationImpl {};
}
