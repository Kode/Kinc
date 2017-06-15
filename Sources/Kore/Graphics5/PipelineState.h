#pragma once

#include <Kore/PipelineState5Impl.h>

#include <Kore/Graphics5/VertexStructure.h>

#include "Graphics.h"

namespace Kore {
	namespace Graphics5 {
		class Shader;

		class PipelineState : public PipelineState5Impl {
		public:
			PipelineState();
			~PipelineState();

			VertexStructure* inputLayout[16];
			Shader* vertexShader;
			Shader* fragmentShader;
			Shader* geometryShader;
			Shader* tessellationControlShader;
			Shader* tessellationEvaluationShader;

			CullMode cullMode;

			bool depthWrite;
			ZCompareMode depthMode;

			ZCompareMode stencilMode;
			StencilAction stencilBothPass;
			StencilAction stencilDepthFail;
			StencilAction stencilFail;
			int stencilReferenceValue;
			int stencilReadMask;
			int stencilWriteMask;

			// One, Zero deactivates blending
			BlendingOperation blendSource;
			BlendingOperation blendDestination;
			//BlendingOperation blendOperation;
			BlendingOperation alphaBlendSource;
			BlendingOperation alphaBlendDestination;
			//BlendingOperation alphaBlendOperation;

			bool colorWriteMaskRed;
			bool colorWriteMaskGreen;
			bool colorWriteMaskBlue;
			bool colorWriteMaskAlpha;

			bool conservativeRasterization;

			void compile();
			ConstantLocation getConstantLocation(const char* name);
			TextureUnit getTextureUnit(const char* name);
		};
	}
}
