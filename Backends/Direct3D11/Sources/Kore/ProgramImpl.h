#pragma once

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11Buffer;

namespace Kore {
	class Shader;

	class ProgramImpl {
	public:
		ID3D11InputLayout* inputLayout;
		ID3D11Buffer* fragmentConstantBuffer;
		ID3D11Buffer* vertexConstantBuffer;
		Shader* vertexShader;
		Shader* fragmentShader;
		static void setConstants();
	};

	class ConstantLocationImpl {
	public:
		bool vertex;
		u8 offset;
		u8 size;
	};
}
