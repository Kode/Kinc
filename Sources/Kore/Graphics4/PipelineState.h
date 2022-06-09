#pragma once

#include <kinc/graphics4/pipeline.h>

#include "Graphics.h"

namespace Kore {
	namespace Graphics4 {
		class Shader;
		class VertexStructure;

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

			ZCompareMode stencilFrontMode;
			StencilAction stencilFrontBothPass;
			StencilAction stencilFrontDepthFail;
			StencilAction stencilFrontFail;

			ZCompareMode stencilBackMode;
			StencilAction stencilBackBothPass;
			StencilAction stencilBackDepthFail;
			StencilAction stencilBackFail;

			int stencilReferenceValue;
			int stencilReadMask;
			int stencilWriteMask;

			// One, Zero deactivates blending
			BlendingFactor blendSource;
			BlendingFactor blendDestination;
			BlendingOperation blendOperation;
			BlendingFactor alphaBlendSource;
			BlendingFactor alphaBlendDestination;
			BlendingOperation alphaBlendOperation;

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

			kinc_g4_pipeline_t kincPipeline;
		};
	}
}
