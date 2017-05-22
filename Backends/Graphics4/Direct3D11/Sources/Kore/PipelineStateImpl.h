#pragma once

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11Buffer;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11BlendState;

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
		ID3D11DepthStencilState* depthStencilState;
		ID3D11RasterizerState* rasterizerState;
		ID3D11RasterizerState* rasterizerStateScissor;
		ID3D11BlendState* blendState;
		void set(Graphics4::PipelineState* pipeline, bool scissoring);
		void setRasterizerState(bool scissoring);
		static void setConstants();
	};
}
