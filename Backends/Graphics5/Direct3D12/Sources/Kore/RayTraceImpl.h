#pragma once

#ifdef KORE_DXR

#include <d3d12.h>
#include "D3D12RaytracingFallback.h"
#include "D3D12RaytracingPrototypeHelpers.hpp"

namespace Kore {

	namespace Graphics5 {

		class RayTracePipelineImpl {
		public:
			ID3D12RaytracingFallbackStateObject* fallbackState;
			ID3D12StateObjectPrototype* dxrState;
			ID3D12Resource* rayGenShaderTable;
			ID3D12Resource* hitGroupShaderTable;
			ID3D12Resource* missShaderTable;
		};

		class AccelerationStructureImpl {
		public:
			ID3D12Resource* bottomLevelAccelerationStructure;
			ID3D12Resource* topLevelAccelerationStructure;
			WRAPPED_GPU_POINTER fallbackTopLevelAccelerationStructurePointer;
		};

		class RayTraceTargetImpl {
		public:
			ID3D12Resource* _texture;
			D3D12_GPU_DESCRIPTOR_HANDLE _textureHandle;
		};

		void enableRaytracing();
	}
}

#endif
