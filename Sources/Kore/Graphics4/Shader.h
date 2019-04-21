#pragma once

#include <Kinc/Graphics4/ConstantLocation.h>
#include <Kinc/Graphics4/Shader.h>

#include "Texture.h"
#include "VertexStructure.h"
#include <Kore/Math/Matrix.h>
#include <Kore/PipelineStateImpl.h>
#include <Kore/ShaderImpl.h>

namespace Kore {
	namespace Graphics4 {
		enum ShaderType { FragmentShader, VertexShader, GeometryShader, TessellationControlShader, TessellationEvaluationShader };

		class Shader {
		public:
			Shader(void* data, int length, ShaderType type);
			Shader(const char* source, ShaderType type); // Beware, this is not portable
			
			Kinc_G4_Shader kincShader;
		private:
			void parse(void* data, int length, ShaderType type);
		};

		class ConstantLocation {
		public:
			ConstantLocation() {

			}

			Kinc_G4_ConstantLocation kincConstant;
		};
	}
}
