#pragma once

struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;

namespace Kore {
	namespace Graphics5 {
		class Shader;
	}

	class Program5Impl {
	public:
		Program5Impl();
		ID3D12PipelineState* pso;
		// ID3D11InputLayout* inputLayout;
		// ID3D11Buffer* fragmentConstantBuffer;
		// ID3D11Buffer* vertexConstantBuffer;
		// ID3D11Buffer* geometryConstantBuffer;
		// ID3D11Buffer* tessEvalConstantBuffer;
		// ID3D11Buffer* tessControlConstantBuffer;

		Graphics5::Shader* vertexShader;
		Graphics5::Shader* fragmentShader;
		Graphics5::Shader* geometryShader;
		Graphics5::Shader* tessEvalShader;
		Graphics5::Shader* tessControlShader;
		static void setConstants();
		static Program5Impl* _current;
	};

	class ConstantLocation5Impl {
	public:
		u8 vertexOffset;
		u8 vertexSize;
		u8 fragmentOffset;
		u8 fragmentSize;
		u8 geometryOffset;
		u8 geometrySize;
		u8 tessEvalOffset;
		u8 tessEvalSize;
		u8 tessControlOffset;
		u8 tessControlSize;
	};

	class AttributeLocation5Impl {};
}
