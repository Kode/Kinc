#pragma once

namespace Kore {
	namespace Graphics5 {
		class PipelineState;
		class Shader;
	}

	class PipelineState5Impl {
	public:
		PipelineState5Impl();
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
