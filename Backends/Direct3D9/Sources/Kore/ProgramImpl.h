#pragma once

struct IDirect3DVertexDeclaration9;

namespace Kore {
	class Shader;

	class ProgramImpl {
	public:
		~ProgramImpl();
		Shader* vertexShader;
		Shader* fragmentShader;
		IDirect3DVertexDeclaration9* vertexDecleration;
		int halfPixelLocation;
	};
}
