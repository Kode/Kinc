#pragma once

#include <kinc/graphics4/constantlocation.h>
#include <kinc/graphics4/shader.h>

#include "Texture.h"
#include "VertexStructure.h"
#include <Kore/Math/Matrix.h>
#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/backend/graphics4/shader.h>

namespace Kore {
	namespace Graphics4 {
		enum ShaderType { FragmentShader, VertexShader, GeometryShader, TessellationControlShader, TessellationEvaluationShader };

		class Shader {
		public:
			Shader(void *data, int length, ShaderType type);
			Shader(const char *source, ShaderType type); // Beware, this is not portable
			virtual ~Shader();

			kinc_g4_shader_t kincShader;
		};

		class ConstantLocation {
		public:
			ConstantLocation() {}

			kinc_g4_constant_location_t kincConstant;
		};
	}
}
