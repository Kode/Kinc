#pragma once

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11Buffer;

namespace Kore {
	namespace Graphics4 {
		class Shader;
	}

	class ProgramImpl {
	public:
		ProgramImpl();
		ID3D11InputLayout* inputLayout;
		ID3D11Buffer* fragmentConstantBuffer;
		ID3D11Buffer* vertexConstantBuffer;
		ID3D11Buffer* geometryConstantBuffer;
		ID3D11Buffer* tessEvalConstantBuffer;
		ID3D11Buffer* tessControlConstantBuffer;
		Graphics4::Shader* vertexShader;
		Graphics4::Shader* fragmentShader;
		Graphics4::Shader* geometryShader;
		Graphics4::Shader* tessEvalShader;
		Graphics4::Shader* tessControlShader;
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

	class AttributeLocationImpl {};
}
