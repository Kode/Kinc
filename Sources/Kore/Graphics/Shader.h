#pragma once

#include <Kore/Math/Matrix.h>
#include "VertexStructure.h"
#include <Kore/ShaderImpl.h>
#include <Kore/ProgramImpl.h>
#include "Texture.h"

namespace Kore {
	enum ShaderType {
		FragmentShader, VertexShader
	};

	class Shader : public ShaderImpl {
	public:
		Shader(void* source, int length, ShaderType type);
	};

	class ConstantLocation : public ConstantLocationImpl {

	};

	class Program : public ProgramImpl {
	public:
		Program();
		void setVertexShader(Shader* shader);
		void setFragmentShader(Shader* shader);
		void link(const VertexStructure& structure);
		ConstantLocation getConstantLocation(const char* name);
		TextureUnit getTextureUnit(const char* name);
		void set();
	};
}
