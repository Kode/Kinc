#include "pch.h"
#include "RayTraceImpl.h"

#ifdef KORE_DXR

#undef min
#undef max
#include <Kore/Math/Core.h>
#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/ConstantBuffer.h>
#include <Kore/Graphics5/RayTrace.h>
#include <Kore/Graphics5/Graphics.h>
#include <dxgi.h>
#include "d3dx12.h"
#include "Direct3D12.h"

extern ID3D12CommandQueue* commandQueue;

namespace {
	bool useComputeFallback;

	const wchar_t* hitGroupName = L"hitgroup";
	const wchar_t* rayShaderEntry = L"raygeneration";
	const wchar_t* hitShaderEntry = L"closesthit";
	const wchar_t* missShaderEntry = L"miss";

	ID3D12RaytracingFallbackDevice* fallbackDevice;
	ID3D12RaytracingFallbackCommandList* fallbackCommandList;
	ID3D12DeviceRaytracingPrototype* dxrDevice;
	ID3D12CommandListRaytracingPrototype* dxrCommandList;
	ID3D12RootSignature* globalRootSignature;
	ID3D12DescriptorHeap* descriptorHeap;
	UINT descriptorSize;
	Kore::Graphics5::AccelerationStructure* accel;
	Kore::Graphics5::RayTracePipeline* pipeline;
	Kore::Graphics5::RayTraceTarget* output;

	void createRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ID3D12RootSignature** rootSig) {
		ID3DBlob* blob = nullptr;
		ID3DBlob* error = nullptr;
		if (useComputeFallback) {
			fallbackDevice->D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
			fallbackDevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig)));
		}
		else {
			D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
			device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig)));
		}
	}

	void allocateUploadBuffer(ID3D12Device* pDevice, void *pData, UINT64 datasize, ID3D12Resource **ppResource) {
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
		pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(ppResource));
		void *pMappedData;
		(*ppResource)->Map(0, nullptr, &pMappedData);
		memcpy(pMappedData, pData, datasize);
		(*ppResource)->Unmap(0, nullptr);
	}

	void allocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource **ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON) {
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			initialResourceState,
			nullptr,
			IID_PPV_ARGS(ppResource));
	}

	UINT allocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor) {
		static int descriptorsAllocated = 0;
		*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), descriptorsAllocated, descriptorSize);
		return descriptorsAllocated++;
	}
	
	WRAPPED_GPU_POINTER createFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC rawBufferUavDesc = {};
		rawBufferUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		rawBufferUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		rawBufferUavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		rawBufferUavDesc.Buffer.NumElements = bufferNumElements;

		D3D12_CPU_DESCRIPTOR_HANDLE bottomLevelDescriptor;
		int descriptorHeapIndex = allocateDescriptor(&bottomLevelDescriptor);
		device->CreateUnorderedAccessView(resource, nullptr, &rawBufferUavDesc, bottomLevelDescriptor);
		return fallbackDevice->GetWrappedPointerSimple(descriptorHeapIndex, resource->GetGPUVirtualAddress());
	}

	class ShaderRecord {
	public:
		void* shaderId;
		UINT shaderIdSize;
		void* localRootArguments;
		UINT localRootArgumentsSize;

		ShaderRecord(void* pShaderId, UINT _shaderIdSize) :
			shaderId(pShaderId), shaderIdSize(_shaderIdSize),
			localRootArguments(nullptr), localRootArgumentsSize(0) {}
		ShaderRecord(void* pShaderId, UINT _shaderIdSize, void* pLocalRootArguments, UINT _localRootArgumentsSize) :
			shaderId(pShaderId), shaderIdSize(_shaderIdSize),
			localRootArguments(pLocalRootArguments), localRootArgumentsSize(_localRootArgumentsSize) {}

		void copyTo(void* dest) const {
			uint8_t* byteDest = static_cast<uint8_t*>(dest);
			memcpy(byteDest, shaderId, shaderIdSize);
			if (localRootArguments) {
				memcpy(byteDest + shaderIdSize, localRootArguments, localRootArgumentsSize);
			}
		}
	};
	
	class ShaderTable {
	public:
		uint8_t* mappedShaderRecords;
		UINT shaderRecordSize;
		std::vector<ShaderRecord> shaderRecords;
		ID3D12Resource* resource;

		ShaderTable() {}
		~ShaderTable() { if (resource != nullptr) resource->Unmap(0, nullptr); }

		ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT recordSize) {
			int align = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
			shaderRecordSize = (recordSize + (align - 1)) & ~(align - 1);
			shaderRecords.reserve(numShaderRecords);
			UINT bufferSize = numShaderRecords * shaderRecordSize;
			
			auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
			device->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&resource));
			
			CD3DX12_RANGE readRange(0, 0);
			resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedShaderRecords));
		}
		
		void push_back(const ShaderRecord& shaderRecord) {
			shaderRecords.push_back(shaderRecord);
			shaderRecord.copyTo(mappedShaderRecords);
			mappedShaderRecords += shaderRecordSize;
		}
	};
}

namespace Kore {
	namespace Graphics5 {

		RayTracePipeline::RayTracePipeline(CommandList* commandList, void* rayShader, int rayShaderSize, void* hitShader, int hitShaderSize, void* missShader, int missShaderSize, ConstantBuffer* constantBuffer) {
			_constantBuffer = constantBuffer;
			// Descriptor heap
			D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
			// Allocate a heap for 3 descriptors:
			// 2 - bottom and top level acceleration structure fallback wrapped pointers
			// 1 - raytracing output texture SRV
			descriptorHeapDesc.NumDescriptors = 3; 
			descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descriptorHeapDesc.NodeMask = 0;
			device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
			descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Device
			if (useComputeFallback) {
				D3D12CreateRaytracingFallbackDevice(device, CreateRaytracingFallbackDeviceFlags::None, 0, IID_PPV_ARGS(&fallbackDevice));
				fallbackDevice->QueryRaytracingCommandList(commandList->_commandList, IID_PPV_ARGS(&fallbackCommandList));
			}
			else {
				device->QueryInterface(IID_PPV_ARGS(&dxrDevice));
				commandList->_commandList->QueryInterface(IID_PPV_ARGS(&dxrCommandList));
			}

			// Root signatures
			// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
			{
				CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
				UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
				CD3DX12_ROOT_PARAMETER rootParameters[3];
				// output view
				rootParameters[0].InitAsDescriptorTable(1, &UAVDescriptor);
				// acceleration structure
				rootParameters[1].InitAsShaderResourceView(0);
				// constant buffer
				rootParameters[2].InitAsConstantBufferView(0);
				
				CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
				createRootSignature(globalRootSignatureDesc, &globalRootSignature);
			}

			// Pipeline
			CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

			auto rayLib = raytracingPipeline.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
			auto hitLib = raytracingPipeline.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
			auto missLib = raytracingPipeline.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
			rayLib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(rayShader, rayShaderSize));
			hitLib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(hitShader, hitShaderSize));
			missLib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(missShader, missShaderSize));
			rayLib->DefineExport(rayShaderEntry);
			hitLib->DefineExport(hitShaderEntry);
			missLib->DefineExport(missShaderEntry);
		
			auto hitGroup = raytracingPipeline.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
			hitGroup->SetClosestHitShaderImport(hitShaderEntry);
			hitGroup->SetHitGroupExport(hitGroupName);
			
			// auto shaderConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
			// UINT payloadSize = 4 * sizeof(float);   // float4 color
			// UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
			// shaderConfig->Config(payloadSize, attributeSize);

			auto globalRootSignatureSubobject = raytracingPipeline.CreateSubobject<CD3D12_ROOT_SIGNATURE_SUBOBJECT>();
			globalRootSignatureSubobject->SetRootSignature(globalRootSignature);

			auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
			UINT maxRecursionDepth = 1; // ~ primary rays only
			pipelineConfig->Config(maxRecursionDepth);

			if (useComputeFallback) {
				fallbackDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&fallbackState));
			}
			else {
				dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&dxrState));
			}

			// Shader tables
			void* rayGenShaderId;
			void* hitGroupShaderId;
			void* missShaderId;
			auto getShaderIds = [&](auto* stateObjectProperties) {
				rayGenShaderId = stateObjectProperties->GetShaderIdentifier(rayShaderEntry);
				hitGroupShaderId = stateObjectProperties->GetShaderIdentifier(hitGroupName);
				missShaderId = stateObjectProperties->GetShaderIdentifier(missShaderEntry);
			};

			// Get shader identifiers
			UINT shaderIdSize;
			if (useComputeFallback) {
				getShaderIds(fallbackState);
				shaderIdSize = fallbackDevice->GetShaderIdentifierSize();
			}
			else {
				ID3D12StateObjectPropertiesPrototype* stateObjectProperties = nullptr;
				getShaderIds(stateObjectProperties);
				shaderIdSize = dxrDevice->GetShaderIdentifierSize();
			}

			// Ray gen shader table
			{
				UINT numShaderRecords = 1;
				UINT shaderRecordSize = shaderIdSize + constantBuffer->size();
				ShaderTable _rayGenShaderTable(device, numShaderRecords, shaderRecordSize);
				
				D3D12_RANGE range;
				range.Begin = 0;
				range.End = constantBuffer->size();
				void* constantBufferData;
				constantBuffer->_buffer->Map(0, &range, (void**)&constantBufferData);
				// constantBuffer->_buffer->Unmap(0, &range);

				_rayGenShaderTable.push_back(ShaderRecord(rayGenShaderId, shaderIdSize, constantBufferData, constantBuffer->size()));
				rayGenShaderTable = _rayGenShaderTable.resource;
			}

			// Hit group shader table
			{
				UINT numShaderRecords = 1;
				UINT shaderRecordSize = shaderIdSize;
				ShaderTable _hitGroupShaderTable(device, numShaderRecords, shaderRecordSize);
				_hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderId, shaderIdSize));
				hitGroupShaderTable = _hitGroupShaderTable.resource;
			}

			// Miss shader table
			{
				UINT numShaderRecords = 1;
				UINT shaderRecordSize = shaderIdSize;
				ShaderTable _missShaderTable(device, numShaderRecords, shaderRecordSize);
				_missShaderTable.push_back(ShaderRecord(missShaderId, shaderIdSize));
				missShaderTable = _missShaderTable.resource;
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
			geometryDesc.Triangles.Transform = 0;
			geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			geometryDesc.Triangles.VertexCount = vb->count();
			geometryDesc.Triangles.VertexBuffer.StartAddress = vb->uploadBuffer->GetGPUVirtualAddress();
			geometryDesc.Triangles.VertexBuffer.StrideInBytes = vb->uploadBuffer->GetDesc().Width / vb->count();
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			// Get required sizes for an acceleration structure
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
			D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
			prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			prebuildInfoDesc.Flags = buildFlags;
			prebuildInfoDesc.NumDescs = 1;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
			prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			prebuildInfoDesc.pGeometryDescs = nullptr;
			if (useComputeFallback) {
				fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &topLevelPrebuildInfo);
			}
			else {
				dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &topLevelPrebuildInfo);
			}

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
			prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			prebuildInfoDesc.pGeometryDescs = &geometryDesc;
			if (useComputeFallback) {
				fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &bottomLevelPrebuildInfo);
			}
			else {
				dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &bottomLevelPrebuildInfo);
			}

			ID3D12Resource* scratchResource;
			allocateUAVBuffer(device, Kore::max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			// Allocate resources for acceleration structures
			// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS.
			{
				D3D12_RESOURCE_STATES initialResourceState;
				if (useComputeFallback) {
					initialResourceState = fallbackDevice->GetAccelerationStructureResourceState();
				}
				else {
					initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
				}
				allocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAccelerationStructure, initialResourceState);
				allocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAccelerationStructure, initialResourceState);
			}

			// Create an instance desc for the bottom-level acceleration structure
			ID3D12Resource* instanceDescs;
			if (useComputeFallback) {
				D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDesc = {};
				instanceDesc.Transform[0] = instanceDesc.Transform[5] = instanceDesc.Transform[10] = 1;
				instanceDesc.InstanceMask = 1;
				UINT numBufferElements = static_cast<UINT>(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
				instanceDesc.AccelerationStructure = createFallbackWrappedPointer(bottomLevelAccelerationStructure, numBufferElements); 
				allocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs);
			}
			else {
				D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
				instanceDesc.Transform[0] = instanceDesc.Transform[5] = instanceDesc.Transform[10] = 1;
				instanceDesc.InstanceMask = 1;
				instanceDesc.AccelerationStructure = bottomLevelAccelerationStructure->GetGPUVirtualAddress();
				allocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs);
			}

			// Create a wrapped pointer to the acceleration structure
			if (useComputeFallback) {
				UINT numBufferElements = static_cast<UINT>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
				fallbackTopLevelAccelerationStructurePointer = createFallbackWrappedPointer(topLevelAccelerationStructure, numBufferElements); 
			}

			// Bottom Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
			{
				bottomLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
				bottomLevelBuildDesc.Flags = buildFlags;
				bottomLevelBuildDesc.ScratchAccelerationStructureData = { scratchResource->GetGPUVirtualAddress(), scratchResource->GetDesc().Width };
				bottomLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
				bottomLevelBuildDesc.DestAccelerationStructureData = { bottomLevelAccelerationStructure->GetGPUVirtualAddress(), bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes };
				bottomLevelBuildDesc.NumDescs = 1;
				bottomLevelBuildDesc.pGeometryDescs = &geometryDesc;
			}

			// Top Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = bottomLevelBuildDesc;
			{
				topLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
				topLevelBuildDesc.DestAccelerationStructureData = { topLevelAccelerationStructure->GetGPUVirtualAddress(), topLevelPrebuildInfo.ResultDataMaxSizeInBytes };
				topLevelBuildDesc.NumDescs = 1;
				topLevelBuildDesc.pGeometryDescs = nullptr;
				topLevelBuildDesc.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
				topLevelBuildDesc.ScratchAccelerationStructureData = { scratchResource->GetGPUVirtualAddress(), scratchResource->GetDesc().Width };
			}

			auto buildAccelerationStructure = [&](auto* raytracingCommandList) {
				raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
				commandList->_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAccelerationStructure));
				raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
			};

			// Build acceleration structure
			if (useComputeFallback) {
				// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer
				ID3D12DescriptorHeap *pDescriptorHeaps[] = { descriptorHeap };
				fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
				buildAccelerationStructure(fallbackCommandList);
			}
			else {
				buildAccelerationStructure(dxrCommandList);
			}

			commandList->_commandList->Close();
			ID3D12CommandList* commandLists[] = {commandList->_commandList}; 
			commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);

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
			int descriptorHeapIndex = allocateDescriptor(&uavDescriptorHandle);
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			device->CreateUnorderedAccessView(_texture, nullptr, &UAVDesc, uavDescriptorHandle);
			
			int descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			_textureHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorHeapIndex, descriptorSize);
		}

		void enableRaytracing() {
			IDXGIAdapter* adapter = nullptr;
			ID3D12Device* testDevice;
			ID3D12DeviceRaytracingPrototype* testRaytracingDevice;
			UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels, D3D12RaytracingPrototype };
			bool isDxrSupported =
				   SUCCEEDED(D3D12EnableExperimentalFeatures(2, experimentalFeatures, nullptr, nullptr))
				&& SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)))
				&& SUCCEEDED(testDevice->QueryInterface(IID_PPV_ARGS(&testRaytracingDevice)));

			// Fallback to compute
			if (!isDxrSupported) {
				useComputeFallback = true;
				ID3D12Device* testDevice;
				UUID experimentalFeatures[] = { D3D12ExperimentalShaderModels };
				D3D12EnableExperimentalFeatures(1, experimentalFeatures, nullptr, nullptr);
				D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice));
			}
		}

		void setAccelerationStructure(AccelerationStructure* _accel) { accel = _accel; }
		void setRayTracePipeline(RayTracePipeline* _pipeline) { pipeline = _pipeline; }
		void setRayTraceTarget(RayTraceTarget* _output) { output = _output; }
		
		void dispatchRays(CommandList* commandList) {
			auto dispatchRaysCommand = [&](auto* commandList, auto* stateObject, auto* dispatchDesc) {
				// Since each shader table has only one shader record, the stride is same as the size.
				dispatchDesc->HitGroupTable.StartAddress = pipeline->hitGroupShaderTable->GetGPUVirtualAddress();
				dispatchDesc->HitGroupTable.SizeInBytes = pipeline->hitGroupShaderTable->GetDesc().Width;
				dispatchDesc->HitGroupTable.StrideInBytes = dispatchDesc->HitGroupTable.SizeInBytes;
				dispatchDesc->MissShaderTable.StartAddress = pipeline->missShaderTable->GetGPUVirtualAddress();
				dispatchDesc->MissShaderTable.SizeInBytes = pipeline->missShaderTable->GetDesc().Width;
				dispatchDesc->MissShaderTable.StrideInBytes = dispatchDesc->MissShaderTable.SizeInBytes;
				dispatchDesc->RayGenerationShaderRecord.StartAddress = pipeline->rayGenShaderTable->GetGPUVirtualAddress();
				dispatchDesc->RayGenerationShaderRecord.SizeInBytes = pipeline->rayGenShaderTable->GetDesc().Width;
				dispatchDesc->Width = output->_width;
				dispatchDesc->Height = output->_height;
				commandList->DispatchRays(stateObject, dispatchDesc);
			};

			commandList->_commandList->SetComputeRootSignature(globalRootSignature);

			// Bind the heaps, acceleration structure and dispatch rays  
			if (useComputeFallback) {
				D3D12_FALLBACK_DISPATCH_RAYS_DESC dispatchDesc = {};
				fallbackCommandList->SetDescriptorHeaps(1, &descriptorHeap);
				// output view
				commandList->_commandList->SetComputeRootDescriptorTable(0, output->_textureHandle);
				// acceleration structure
				fallbackCommandList->SetTopLevelAccelerationStructure(1, accel->fallbackTopLevelAccelerationStructurePointer);
				// Copy the updated constant buffer to GPU
				auto cbGpuAddress = pipeline->_constantBuffer->_buffer->GetGPUVirtualAddress();
				commandList->_commandList->SetComputeRootConstantBufferView(2, cbGpuAddress);
				dispatchRaysCommand(fallbackCommandList, pipeline->fallbackState, &dispatchDesc);
			}
			else {
				D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
				commandList->_commandList->SetDescriptorHeaps(1, &descriptorHeap);
				commandList->_commandList->SetComputeRootDescriptorTable(0, output->_textureHandle);
				commandList->_commandList->SetComputeRootShaderResourceView(1, accel->topLevelAccelerationStructure->GetGPUVirtualAddress());
				auto cbGpuAddress = pipeline->_constantBuffer->_buffer->GetGPUVirtualAddress();
				commandList->_commandList->SetComputeRootConstantBufferView(2, cbGpuAddress);
				dispatchRaysCommand(dxrCommandList, pipeline->dxrState, &dispatchDesc);
			}
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
