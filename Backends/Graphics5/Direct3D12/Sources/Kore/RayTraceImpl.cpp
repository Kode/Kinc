#include "RayTraceImpl.h"
#include "pch.h"

#ifdef KORE_DXR

#undef min
#undef max
#include "Direct3D12.h"
#include "d3dx12.h"
#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/ConstantBuffer.h>
#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics5/RayTrace.h>

extern ID3D12CommandQueue* commandQueue;

namespace {
	const wchar_t* hitGroupName = L"hitgroup";
	const wchar_t* rayGenerationShaderName = L"raygeneration";
	const wchar_t* closestHitShaderName = L"closesthit";
	const wchar_t* missShaderName = L"miss";

	ID3D12Device5* dxrDevice;
	ID3D12GraphicsCommandList4* dxrCommandList;
	ID3D12RootSignature* globalRootSignature;
	ID3D12DescriptorHeap* descriptorHeap;
	UINT descriptorSize;
	Kore::Graphics5::AccelerationStructure* accel;
	Kore::Graphics5::RayTracePipeline* pipeline;
	Kore::Graphics5::RayTraceTarget* output;
}

namespace Kore {
	namespace Graphics5 {

		RayTracePipeline::RayTracePipeline(CommandList* commandList, void* rayTraceShader, int rayTraceShaderSize, ConstantBuffer* constantBuffer) {
			_constantBuffer = constantBuffer;
			// Descriptor heap
			D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
			// Allocate a heap for 3 descriptors:
			// 2 - bottom and top level acceleration structure
			// 1 - raytracing output texture SRV
			descriptorHeapDesc.NumDescriptors = 3;
			descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descriptorHeapDesc.NodeMask = 0;
			device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
			descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Device
			device->QueryInterface(IID_PPV_ARGS(&dxrDevice));
			commandList->_commandList->QueryInterface(IID_PPV_ARGS(&dxrCommandList));

			// Root signatures
			// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
			CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
			UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
			CD3DX12_ROOT_PARAMETER rootParameters[3];
			// Output view
			rootParameters[0].InitAsDescriptorTable(1, &UAVDescriptor);
			// Acceleration structure
			rootParameters[1].InitAsShaderResourceView(0);
			// Constant buffer
			rootParameters[2].InitAsConstantBufferView(0);

			CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
			ID3DBlob* blob = nullptr;
			ID3DBlob* error = nullptr;
			D3D12SerializeRootSignature(&globalRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
			device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&globalRootSignature));

			// Pipeline
			CD3D12_STATE_OBJECT_DESC raytracingPipeline{D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE};

			auto lib = raytracingPipeline.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
			lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(rayTraceShader, rayTraceShaderSize));
			lib->DefineExport(rayGenerationShaderName);
			lib->DefineExport(closestHitShaderName);
			lib->DefineExport(missShaderName);

			auto hitGroup = raytracingPipeline.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
			hitGroup->SetClosestHitShaderImport(closestHitShaderName);
			hitGroup->SetHitGroupExport(hitGroupName);
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

			auto shaderConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
			UINT payloadSize = 4 * sizeof(float);   // float4 color
			UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
			shaderConfig->Config(payloadSize, attributeSize);

			auto globalRootSignatureSubobject = raytracingPipeline.CreateSubobject<CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
			globalRootSignatureSubobject->SetRootSignature(globalRootSignature);

			auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
			UINT maxRecursionDepth = 1; // ~ primary rays only
			pipelineConfig->Config(maxRecursionDepth);

			dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&dxrState));

			// Shader tables
			// Get shader identifiers
			ID3D12StateObjectProperties* stateObjectProps;
			dxrState->QueryInterface(IID_PPV_ARGS(&stateObjectProps));
			void* rayGenShaderId = stateObjectProps->GetShaderIdentifier(rayGenerationShaderName);
			void* missShaderId = stateObjectProps->GetShaderIdentifier(missShaderName);
			void* hitGroupShaderId = stateObjectProps->GetShaderIdentifier(hitGroupName);
			UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
			int align = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

			// Ray gen shader table
			{
				UINT size = shaderIdSize + constantBuffer->size();
				UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
				auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderRecordSize);
				auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&rayGenShaderTable));

				uint8_t* byteDest;
				rayGenShaderTable->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&byteDest));
				void* constantBufferData;
				constantBuffer->_buffer->Map(0, &CD3DX12_RANGE(0, constantBuffer->size()), (void**)&constantBufferData);
				memcpy(byteDest, rayGenShaderId, size);
				memcpy(byteDest + size, constantBufferData, constantBuffer->size());
				rayGenShaderTable->Unmap(0, nullptr);
			}

			// Miss shader table
			{
				UINT size = shaderIdSize;
				UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
				auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderRecordSize);
				auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&missShaderTable));

				uint8_t* byteDest;
				missShaderTable->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&byteDest));
				memcpy(byteDest, missShaderId, size);
				missShaderTable->Unmap(0, nullptr);
			}

			// Hit group shader table
			{
				UINT size = shaderIdSize;
				UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
				auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderRecordSize);
				auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&hitGroupShaderTable));

				uint8_t* byteDest;
				hitGroupShaderTable->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&byteDest));
				memcpy(byteDest, hitGroupShaderId, size);
				hitGroupShaderTable->Unmap(0, nullptr);
			}
		}

		AccelerationStructure::AccelerationStructure(CommandList* commandList, VertexBuffer* vb, IndexBuffer* ib) {
			// Reset the command list for the acceleration structure construction
			commandList->_commandList->Reset(commandList->_commandAllocator, nullptr);

			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
			geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			geometryDesc.Triangles.IndexBuffer = ib->uploadBuffer->GetGPUVirtualAddress();
			geometryDesc.Triangles.IndexCount = ib->count();
			geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			geometryDesc.Triangles.Transform3x4 = 0;
			geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			geometryDesc.Triangles.VertexCount = vb->count();
			geometryDesc.Triangles.VertexBuffer.StartAddress = vb->uploadBuffer->GetGPUVirtualAddress();
			geometryDesc.Triangles.VertexBuffer.StrideInBytes = vb->uploadBuffer->GetDesc().Width / vb->count();
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			// Get required sizes for an acceleration structure
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
			topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			topLevelInputs.Flags = buildFlags;
			topLevelInputs.NumDescs = 1;
			topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
			dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
			bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			bottomLevelInputs.pGeometryDescs = &geometryDesc;
			dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

			ID3D12Resource* scratchResource;
			{
				UINT64 tlSize = topLevelPrebuildInfo.ScratchDataSizeInBytes;
				UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
				auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tlSize > blSize ? tlSize : blSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&scratchResource));
			}

			// Allocate resources for acceleration structures
			// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS.
			{
				auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&bottomLevelAccelerationStructure));
			}
			{
				auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&topLevelAccelerationStructure));
			}

			// Create an instance desc for the bottom-level acceleration structure
			ID3D12Resource* instanceDescs;
			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
			instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = bottomLevelAccelerationStructure->GetGPUVirtualAddress();

			auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(instanceDesc));
			device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instanceDescs));
			void* mappedData;
			instanceDescs->Map(0, nullptr, &mappedData);
			memcpy(mappedData, &instanceDesc, sizeof(instanceDesc));
			instanceDescs->Unmap(0, nullptr);

			// Bottom Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
			bottomLevelBuildDesc.Inputs = bottomLevelInputs;
			bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
			bottomLevelBuildDesc.DestAccelerationStructureData = bottomLevelAccelerationStructure->GetGPUVirtualAddress();

			// Top Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = bottomLevelBuildDesc;
			topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
			topLevelBuildDesc.Inputs = topLevelInputs;
			topLevelBuildDesc.DestAccelerationStructureData = topLevelAccelerationStructure->GetGPUVirtualAddress();
			topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

			// Build acceleration structure
			dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
			commandList->_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAccelerationStructure));
			dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

			commandList->_commandList->Close();
			ID3D12CommandList* commandLists[] = {commandList->_commandList};
			commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);

			// Wait for GPU to finish
			// commandQueue->Signal(frameFences[currentBackBuffer], fenceValues[currentBackBuffer]);
			// frameFences[currentBackBuffer]->SetEventOnCompletion(fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);
			// WaitForSingleObjectEx(frameFenceEvents[currentBackBuffer], INFINITE, FALSE);
			// fenceValues[currentBackBuffer]++;
		}

		RayTraceTarget::RayTraceTarget(int width, int height) : _width(width), _height(height) {
			auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			device->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&_texture));

			D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
			static int descriptorsAllocated = 0;
			uavDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), descriptorsAllocated, descriptorSize);
			int descriptorHeapIndex = descriptorsAllocated++;

			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			device->CreateUnorderedAccessView(_texture, nullptr, &UAVDesc, uavDescriptorHandle);

			int descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			_textureHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorHeapIndex, descriptorSize);
		}

		void setAccelerationStructure(AccelerationStructure* _accel) {
			accel = _accel;
		}
		void setRayTracePipeline(RayTracePipeline* _pipeline) {
			pipeline = _pipeline;
		}
		void setRayTraceTarget(RayTraceTarget* _output) {
			output = _output;
		}

		void dispatchRays(CommandList* commandList) {
			commandList->_commandList->SetComputeRootSignature(globalRootSignature);

			// Bind the heaps, acceleration structure and dispatch rays
			commandList->_commandList->SetDescriptorHeaps(1, &descriptorHeap);
			commandList->_commandList->SetComputeRootDescriptorTable(0, output->_textureHandle);
			commandList->_commandList->SetComputeRootShaderResourceView(1, accel->topLevelAccelerationStructure->GetGPUVirtualAddress());
			auto cbGpuAddress = pipeline->_constantBuffer->_buffer->GetGPUVirtualAddress();
			commandList->_commandList->SetComputeRootConstantBufferView(2, cbGpuAddress);
			
			// Since each shader table has only one shader record, the stride is same as the size.
			D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
			dispatchDesc.HitGroupTable.StartAddress = pipeline->hitGroupShaderTable->GetGPUVirtualAddress();
			dispatchDesc.HitGroupTable.SizeInBytes = pipeline->hitGroupShaderTable->GetDesc().Width;
			dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
			dispatchDesc.MissShaderTable.StartAddress = pipeline->missShaderTable->GetGPUVirtualAddress();
			dispatchDesc.MissShaderTable.SizeInBytes = pipeline->missShaderTable->GetDesc().Width;
			dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
			dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline->rayGenShaderTable->GetGPUVirtualAddress();
			dispatchDesc.RayGenerationShaderRecord.SizeInBytes = pipeline->rayGenShaderTable->GetDesc().Width;
			dispatchDesc.Width = output->_width;
			dispatchDesc.Height = output->_height;
			dispatchDesc.Depth = 1;
			dxrCommandList->SetPipelineState1(pipeline->dxrState);
			dxrCommandList->DispatchRays(&dispatchDesc);
		}

		void copyRayTraceTarget(CommandList* commandList, RenderTarget* renderTarget, RayTraceTarget* output) {
			D3D12_RESOURCE_BARRIER preCopyBarriers[2];
			preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget->renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
			preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(output->_texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			commandList->_commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

			commandList->_commandList->CopyResource(renderTarget->renderTarget, output->_texture);

			D3D12_RESOURCE_BARRIER postCopyBarriers[2];
			postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget->renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
			postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(output->_texture, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandList->_commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
		}
	}
}

#endif // KORE_DXR
