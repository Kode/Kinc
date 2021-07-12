#pragma once

#include <kinc/graphics5/pipeline.h>

#include <Kore/Graphics5/VertexStructure.h>

#include "Graphics.h"

namespace Kore {
	namespace Graphics5 {
		class Shader;

		class PipelineState {
		public:
			PipelineState();
			~PipelineState();

			VertexStructure *inputLayout[16];
			Shader *vertexShader;
			Shader *fragmentShader;
			Shader *geometryShader;
			Shader *tessellationControlShader;
			Shader *tessellationEvaluationShader;

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
			// BlendingOperation blendOperation;
			BlendingOperation alphaBlendSource;
			BlendingOperation alphaBlendDestination;
			// BlendingOperation alphaBlendOperation;

			bool colorWriteMaskRed[8]; // Per render target
			bool colorWriteMaskGreen[8];
			bool colorWriteMaskBlue[8];
			bool colorWriteMaskAlpha[8];

			int colorAttachmentCount;
			RenderTargetFormat colorAttachment[8];

			int depthAttachmentBits;
			int stencilAttachmentBits;

			bool conservativeRasterization;

			void compile();
			ConstantLocation getConstantLocation(const char *name);
			TextureUnit getTextureUnit(const char *name);

			kinc_g5_pipeline_t kincPipeline;
		};
	}
}
