#pragma once

namespace Kore {
	class Shader;

	class ProgramImpl {
	protected:
		uint programId;
		Shader* vertexShader;
		Shader* fragmentShader;
		Shader* geometryShader;
		Shader* tesselationControlShader;
		Shader* tesselationEvaluationShader;

		ProgramImpl();
		virtual ~ProgramImpl();
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
