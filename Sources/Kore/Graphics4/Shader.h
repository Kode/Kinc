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
			virtual ~Shader();
			
			kinc_g4_shader_t kincShader;
		};

		class ConstantLocation {
		public:
			ConstantLocation() {

			}

			kinc_g4_constant_location_t kincConstant;
		};
	}
}
