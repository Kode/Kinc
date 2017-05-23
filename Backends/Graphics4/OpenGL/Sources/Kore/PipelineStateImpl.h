#pragma once

namespace Kore {
	namespace Graphics4 {
		class PipelineState;
		class Shader;
	}

	class PipelineStateImpl {
	public:
		uint programId;
		Graphics4::Shader* vertexShader;
		Graphics4::Shader* fragmentShader;
		Graphics4::Shader* geometryShader;
		Graphics4::Shader* tessellationControlShader;
		Graphics4::Shader* tessellationEvaluationShader;

		PipelineStateImpl();
		virtual ~PipelineStateImpl();
		int findTexture(const char* name);
		char** textures;
		int* textureValues;
		int textureCount;
		void set(Graphics4::PipelineState* pipeline);
	};
}
