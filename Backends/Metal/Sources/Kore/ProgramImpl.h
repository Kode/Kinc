#pragma once

namespace Kore {
	class Shader;

	class ProgramImpl {
	protected:
		Shader* vertexShader;
		Shader* fragmentShader;

		ProgramImpl();
	};

	class ConstantLocationImpl {
	public:
		
	};
}
