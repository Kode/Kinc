#pragma once

namespace Kore {
	namespace Graphics4 {
		class ConstantLocation;
		class PipelineState;
	}

	namespace Graphics5 {
		class PipelineState;
		class Shader;
	}

	class PipelineState5Impl {
	public:
		PipelineState5Impl();
		Graphics4::PipelineState* state;
	};

	class ConstantLocation5Impl {
	public:
		Graphics4::ConstantLocation* location;
	};

	class AttributeLocation5Impl {};
}
