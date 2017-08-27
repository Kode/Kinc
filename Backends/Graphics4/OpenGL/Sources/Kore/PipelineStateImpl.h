#pragma once

namespace Kore {
	namespace Graphics4 {
		class PipelineState;
		class Shader;
	}

	class PipelineStateImpl {
	public:
		uint programId;

		PipelineStateImpl();
		virtual ~PipelineStateImpl();
		int findTexture(const char* name);
		char** textures;
		int* textureValues;
		int textureCount;
		void set(Graphics4::PipelineState* pipeline);
	};
}
