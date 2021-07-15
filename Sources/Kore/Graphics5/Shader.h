#pragma once

#include "Texture.h"
#include "VertexStructure.h"
#include <Kore/Math/Matrix.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>

namespace Kore {
	namespace Graphics5 {
		enum ShaderType { FragmentShader, VertexShader, GeometryShader, TessellationControlShader, TessellationEvaluationShader };

		class Shader {
		public:
			Shader(void *source, int length, ShaderType type);
			~Shader();

			kinc_g5_shader_t kincShader;
		};

		class ConstantLocation {
		public:
			kinc_g5_constant_location_t kincConstantLocation;
		};
	}
}
