#pragma once

#ifdef KORE_DXR

#include <d3d12.h>
#include "D3D12RaytracingHelpers.hpp"

namespace Kore {

	namespace Graphics5 {

		class RayTracePipelineImpl {
		public:
			ID3D12StateObject* dxrState;
			ID3D12Resource* rayGenShaderTable;
			ID3D12Resource* missShaderTable;
			ID3D12Resource* hitGroupShaderTable;
		};

		class AccelerationStructureImpl {
		public:
			ID3D12Resource* bottomLevelAccelerationStructure;
			ID3D12Resource* topLevelAccelerationStructure;
		};

		class RayTraceTargetImpl {
		public:
			ID3D12Resource* _texture;
			D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle;
		};
	}
}

#endif
