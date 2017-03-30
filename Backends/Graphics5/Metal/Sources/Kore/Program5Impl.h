#pragma once

#include <objc/runtime.h>

namespace Kore {
	namespace Graphics5 {
		class Shader;
	}

	class Program5Impl {
	protected:
		Graphics5::Shader* vertexShader;
		Graphics5::Shader* fragmentShader;
		id pipeline;
		id reflection;
		Program5Impl();
	};

	class ConstantLocation5Impl {
	public:
		int vertexOffset;
		int fragmentOffset;
	};
}
