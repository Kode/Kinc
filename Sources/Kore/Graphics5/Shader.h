#pragma once

#include "Texture.h"
#include "VertexStructure.h"
#include <Kore/Math/Matrix.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>

namespace Kore {
	namespace Graphics5 {
		enum ShaderType { FragmentShader, VertexShader, GeometryShader, TessellationControlShader, TessellationEvaluationShader };

		class Shader : public Shader5Impl {
		public:
			Shader(void *source, int length, ShaderType type);
		};

		class ConstantLocation : public ConstantLocation5Impl {};
	}
}
