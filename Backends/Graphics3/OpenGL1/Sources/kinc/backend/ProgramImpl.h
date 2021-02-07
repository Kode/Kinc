#pragma once

namespace Kore {
	namespace Graphics4 {
		class Shader;
	}

	class ProgramImpl {
	protected:
		uint programId;
		Graphics4::Shader* vertexShader;
		Graphics4::Shader* fragmentShader;
		Graphics4::Shader* geometryShader;
		Graphics4::Shader* tessellationControlShader;
		Graphics4::Shader* tessellationEvaluationShader;

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
