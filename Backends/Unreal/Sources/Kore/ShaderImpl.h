#pragma once

#include "ShaderParameters.h"

namespace Kore {
	class ConstantLocationImpl {
	public:
		FShaderParameter parameter;
	};

	class AttributeLocationImpl {};

	class ShaderImpl {
	public:
		void* shader;
	};
}
