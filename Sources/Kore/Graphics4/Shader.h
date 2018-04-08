#pragma once

#include "Texture.h"
#include "VertexStructure.h"
#include <Kore/Math/Matrix.h>
#include <Kore/ShaderImpl.h>
#include <Kore/PipelineStateImpl.h>

namespace Kore {
	namespace Graphics4 {
		enum ShaderType { FragmentShader, VertexShader, GeometryShader, TessellationControlShader, TessellationEvaluationShader };

		class Shader : public ShaderImpl {
		public:
			Shader(void* data, int length, ShaderType type);
			Shader(const char* source, ShaderType type); // Beware, this is not portable
		private:
			void setId() {
				static u32 lastId = 0;
				id = lastId++;
			}
			u32 id;
		};

		class ConstantLocation : public ConstantLocationImpl {
		public:
			u32 id;

			ConstantLocation() {
				static u32 lastId = 0;
				id = lastId++;
			}
		};
	}
}
