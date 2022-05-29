#pragma once

#include <kinc/graphics5/raytrace.h>

namespace Kore {
	namespace Graphics5 {

		class CommandList;
		class ConstantBuffer;
		class RenderTarget;
		class VertexBuffer;
		class IndexBuffer;

		class RayTracePipeline : public RayTracePipelineImpl {
		public:
			RayTracePipeline(CommandList *commandList, void *rayTraceShader, int rayTraceShaderSize, ConstantBuffer *constantBuffer);
			ConstantBuffer *_constantBuffer;
		};

		class AccelerationStructure : public AccelerationStructureImpl {
		public:
			AccelerationStructure(CommandList *commandList, VertexBuffer *vb, IndexBuffer *ib);
		};

		class RayTraceTarget : public RayTraceTargetImpl {
		public:
			RayTraceTarget(int width, int height);
			int _width;
			int _height;
		};

		void setAccelerationStructure(AccelerationStructure *accel);
		void setRayTracePipeline(RayTracePipeline *pipeline);
		void setRayTraceTarget(RayTraceTarget *output);
		void dispatchRays(CommandList *commandList);
		void copyRayTraceTarget(CommandList *commandList, RenderTarget *renderTarget, RayTraceTarget *output);
	}
}
