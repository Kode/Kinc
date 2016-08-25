#pragma once

#include <Kore/ComputeImpl.h>

namespace Kore {
	class ComputeConstantLocation : public ComputeConstantLocationImpl {

	};

	class Texture;

	class ComputeTextureUnit : public ComputeTextureUnitImpl {

	};

	class ComputeShader : public ComputeShaderImpl {
	public:
		ComputeShader(void* source, int length);
		ComputeConstantLocation getConstantLocation(const char* name);
		ComputeTextureUnit getTextureUnit(const char* name);
	};

	namespace Compute {
		void setFloat(ComputeConstantLocation location, float value);
		void setTexture(ComputeTextureUnit unit, Texture* texture);
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
