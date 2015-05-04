#pragma once

#include <objc/runtime.h>

namespace Kore {
	class Shader;

	class ProgramImpl {
	protected:
		Shader* vertexShader;
		Shader* fragmentShader;
		id pipeline;
		id reflection;
		ProgramImpl();
	};

	class ConstantLocationImpl {
	public:
		int vertexOffset;
		int fragmentOffset;
	};
}
