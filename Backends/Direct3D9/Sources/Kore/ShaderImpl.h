#pragma once

#include <map>
#include <string>

namespace Kore {
	struct ShaderRegister {
		u8 regtype;
		u8 regindex;
		u8 regcount;
	};

	class ConstantLocationImpl {
	public:
		ShaderRegister reg;
		int shaderType; //0: Vertex, 1: Fragment
	};

	class AttributeLocationImpl {

	};

	class ShaderImpl {
	public:
		std::map<std::string, ShaderRegister> constants;
		std::map<std::string, int> attributes;
		void* shader;
	};
}
