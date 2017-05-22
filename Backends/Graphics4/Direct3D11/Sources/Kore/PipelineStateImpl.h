#pragma once

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11Buffer;

namespace Kore {
	namespace Graphics4 {
		class PipelineState;
		class Shader;
	}

	class PipelineStateImpl {
	public:
		PipelineStateImpl();
		ID3D11InputLayout* d3d11inputLayout;
		ID3D11Buffer* fragmentConstantBuffer;
		ID3D11Buffer* vertexConstantBuffer;
		ID3D11Buffer* geometryConstantBuffer;
		ID3D11Buffer* tessEvalConstantBuffer;
		ID3D11Buffer* tessControlConstantBuffer;
		void set(Graphics4::PipelineState* pipeline);
		static void setConstants();
	};
}
