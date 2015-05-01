#pragma once

namespace Kore {
	class Shader;

	class ProgramImpl {
	protected:
		uint programId;
		Shader* vertexShader;
		Shader* fragmentShader;
		Shader* geometryShader;
		Shader* tesselationEvaluationShader;
		Shader* tesselationControlShader;

		ProgramImpl();
		int findTexture(const char* name);
		const char** textures;
		int* textureValues;
		int textureCount;
	};

	class ConstantLocationImpl {
	public:
		int location;
	};
}
