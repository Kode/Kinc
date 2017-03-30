#pragma once

struct IDirect3DVertexDeclaration9;

namespace Kore {
	namespace Graphics4 {
		class Shader;
	}

	class ProgramImpl {
	public:
		Graphics4::Shader* vertexShader;
		Graphics4::Shader* fragmentShader;
		IDirect3DVertexDeclaration9* vertexDecleration;
		int halfPixelLocation;
	};
}
