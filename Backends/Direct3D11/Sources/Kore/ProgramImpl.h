#pragma once

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11Buffer;

namespace Kore {
	class Shader;

	class ProgramImpl {
	public:
		ProgramImpl();
		ID3D11InputLayout* inputLayout;
		ID3D11Buffer* fragmentConstantBuffer;
		ID3D11Buffer* vertexConstantBuffer;
		ID3D11Buffer* geometryConstantBuffer;
		ID3D11Buffer* tessEvalConstantBuffer;
		ID3D11Buffer* tessControlConstantBuffer;
		Shader* vertexShader;
		Shader* fragmentShader;
		Shader* geometryShader;
		Shader* tessEvalShader;
		Shader* tessControlShader;
		static void setConstants();
	};

	class ConstantLocationImpl {
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

	class AttributeLocationImpl {

	};
}
