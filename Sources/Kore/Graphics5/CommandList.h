#pragma once

#include <Kore/CommandList5Impl.h>

namespace Kore {
	namespace Graphics5 {
		class ConstantBuffer;
		class IndexBuffer;
		class PipelineState;
		class RenderTarget;
		class Texture;
		class VertexBuffer;

		class CommandList : public CommandList5Impl {
		public:
			CommandList();
			~CommandList();
			void begin();
			void end();
			void clear(RenderTarget* renderTarget, uint flags, uint color = 0, float depth = 1.0f, int stencil = 0);
			void renderTargetToFramebufferBarrier(RenderTarget* renderTarget);
			void framebufferToRenderTargetBarrier(RenderTarget* renderTarget);
			void textureToRenderTargetBarrier(RenderTarget* renderTarget);
			void renderTargetToTextureBarrier(RenderTarget* renderTarget);
			void drawIndexedVertices();
			void drawIndexedVertices(int start, int count);
			void viewport(int x, int y, int width, int height);
			void scissor(int x, int y, int width, int height);
			void disableScissor();
			void setPipeline(PipelineState* pipeline);
			void setVertexBuffers(VertexBuffer** buffers, int* offsets, int count);
			void setIndexBuffer(IndexBuffer& buffer);
			//void restoreRenderTarget();
			void setRenderTargets(RenderTarget** targets, int count);
			void upload(IndexBuffer* buffer);
			void upload(VertexBuffer* buffer);
			void upload(Texture* texture);
			void setVertexConstantBuffer(ConstantBuffer* buffer, int offset);
			void setFragmentConstantBuffer(ConstantBuffer* buffer, int offset);
			void setPipelineLayout();
			void execute();
			void executeAndWait();
		};
	}
}
