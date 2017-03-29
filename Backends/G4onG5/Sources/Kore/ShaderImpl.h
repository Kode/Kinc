#pragma once

#include <Kore/Graphics5/Shader.h>

namespace Kore {
	class ShaderImpl {
	public:
		ShaderImpl(void* data, int length, Graphics5::ShaderType type);
		Graphics5::Shader _shader;
	};
}
