#pragma once

#include <Kore/CommandList5Impl.h>

namespace Kore {
	namespace Graphics5 {
		class IndexBuffer;
		class PipelineState;
		class VertexBuffer;

		class CommandList : public CommandList5Impl {
			CommandList();
			~CommandList();
			void drawIndexedVertices();
			void drawIndexedVertices(int start, int count);
			void viewport(int x, int y, int width, int height);
			void scissor(int x, int y, int width, int height);
			void disableScissor();
			void setPipeline(PipelineState* pipeline);
			void setVertexBuffers(VertexBuffer** buffers, int count);
			void setIndexBuffer(IndexBuffer& buffer);
		};
	}
}
