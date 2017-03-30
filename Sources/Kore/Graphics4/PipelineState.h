#pragma once

#include "PipelineStateBase.h"
//#include "Shader.h"

namespace Kore {
	namespace Graphics4 {
		class PipelineState : public PipelineStateBase, public Program {

		private:
			// Program* program;
			// const char** textures;
			// int* textureValues;

			// int findTexture(char* name);

		public:
			PipelineState();
			// void delete();
			void compile();
			// ConstantLocation getConstantLocation(const char* name);
			// TextureUnit getTextureUnit(const char* name);
		};
	}
}
