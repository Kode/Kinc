#pragma once

#include <Kore/ComputeImpl.h>

namespace Kore {
	class ComputeShader : public ComputeShaderImpl {
	public:
		ComputeShader(void* source, int length);
	};

	namespace Compute {
		//void setTexture(TextureUnit unit, Texture* texture);
	
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
