#pragma once

struct IDirect3DVertexDeclaration9;

namespace Kore {
	namespace Graphics4 {
		class PipelineState;
	}

	class PipelineStateImpl {
	public:
		IDirect3DVertexDeclaration9* vertexDecleration;
		int halfPixelLocation;
		void set(Graphics4::PipelineState* pipeline);
	};
}
