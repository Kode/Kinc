#include "pch.h"

#include "RayTraceImpl.h"

#ifdef KORE_DXR

#undef min
#undef max
#include "Direct3D12.h"
#include "d3dx12.h"
#include "D3D12RaytracingHelpers.hpp"
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/raytrace.h>

extern ID3D12CommandQueue* commandQueue;

const wchar_t* hit_group_name = L"hitgroup";
const wchar_t* raygen_shader_name = L"raygeneration";
const wchar_t* closesthit_shader_name = L"closesthit";
const wchar_t* miss_shader_name = L"miss";

ID3D12Device5* dxrDevice;
ID3D12GraphicsCommandList4* dxrCommandList;
ID3D12RootSignature* globalRootSignature;
ID3D12DescriptorHeap* descriptorHeap;
UINT descriptorSize;
kinc_raytrace_acceleration_structure_t* accel;
kinc_raytrace_pipeline_t* pipeline;
kinc_raytrace_target_t* output;

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list *command_list, void *ray_shader, int ray_shader_size, kinc_g5_constant_buffer_t *constant_buffer) {
	pipeline->_constant_buffer = constant_buffer;
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
	command_list->impl._commandList->QueryInterface(IID_PPV_ARGS(&dxrCommandList));

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
	lib->SetDXILLibrary(&CD3DX12_SHADER_BYTECODE(ray_shader, ray_shader_size));
	lib->DefineExport(raygen_shader_name);
	lib->DefineExport(closesthit_shader_name);
	lib->DefineExport(miss_shader_name);

	auto hitGroup = raytracingPipeline.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(closesthit_shader_name);
	hitGroup->SetHitGroupExport(hit_group_name);
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

	dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&pipeline->impl.dxr_state));

	// Shader tables
	// Get shader identifiers
	ID3D12StateObjectProperties* stateObjectProps;
	pipeline->impl.dxr_state->QueryInterface(IID_PPV_ARGS(&stateObjectProps));
	void* rayGenShaderId = stateObjectProps->GetShaderIdentifier(raygen_shader_name);
	void* missShaderId = stateObjectProps->GetShaderIdentifier(miss_shader_name);
	void* hitGroupShaderId = stateObjectProps->GetShaderIdentifier(hit_group_name);
	UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	int align = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	// Ray gen shader table
	{
		UINT size = shaderIdSize + constant_buffer->impl.mySize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderRecordSize);
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pipeline->impl.raygen_shader_table));

		uint8_t* byteDest;
		pipeline->impl.raygen_shader_table->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void **>(&byteDest));
		void* constantBufferData;
		constant_buffer->impl._buffer->Map(0, &CD3DX12_RANGE(0, constant_buffer->impl.mySize), (void**)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->impl.mySize);
		pipeline->impl.raygen_shader_table->Unmap(0, nullptr);
	}

	// Miss shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderRecordSize);
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pipeline->impl.miss_shader_table));

		uint8_t* byteDest;
		pipeline->impl.miss_shader_table->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void **>(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->Unmap(0, nullptr);
	}

	// Hit group shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderRecordSize);
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pipeline->impl.hitgroup_shader_table));

		uint8_t* byteDest;
		pipeline->impl.hitgroup_shader_table->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void **>(&byteDest));
		memcpy(byteDest, hitGroupShaderId, size);
		pipeline->impl.hitgroup_shader_table->Unmap(0, nullptr);
	}
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list, kinc_g5_vertex_buffer_t *vb, kinc_g5_index_buffer_t *ib) {
	// Reset the command list for the acceleration structure construction
	command_list->impl._commandList->Reset(command_list->impl._commandAllocator, nullptr);

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = ib->impl.uploadBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = ib->impl.myCount;
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = vb->impl.myCount;
	geometryDesc.Triangles.VertexBuffer.StartAddress = vb->impl.uploadBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = vb->impl.uploadBuffer->GetDesc().Width / vb->impl.myCount;
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
		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&accel->impl.bottom_level_accel));
	}
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&accel->impl.top_level_accel));
	}

	// Create an instance desc for the bottom-level acceleration structure
	ID3D12Resource* instanceDescs;
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = accel->impl.bottom_level_accel->GetGPUVirtualAddress();

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
	bottomLevelBuildDesc.DestAccelerationStructureData = accel->impl.bottom_level_accel->GetGPUVirtualAddress();

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = bottomLevelBuildDesc;
	topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs = topLevelInputs;
	topLevelBuildDesc.DestAccelerationStructureData = accel->impl.top_level_accel->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

	// Build acceleration structure
	dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	command_list->impl._commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(accel->impl.bottom_level_accel));
	dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	kinc_g5_command_list_execute_and_wait(command_list);
}

void kinc_raytrace_target_init(kinc_raytrace_target_t *target, int width, int height) {
	target->_width = width;
	target->_height = height;

	auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&target->impl._texture));

	D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
	static int descriptorsAllocated = 0;
	uavDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), descriptorsAllocated, descriptorSize);
	int descriptorHeapIndex = descriptorsAllocated++;

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(target->impl._texture, nullptr, &UAVDesc, uavDescriptorHandle);

	int descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	target->impl._texture_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorHeapIndex, descriptorSize);
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_raytrace_target_t *_output) {
	output = _output;
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	command_list->impl._commandList->SetComputeRootSignature(globalRootSignature);

	// Bind the heaps, acceleration structure and dispatch rays
	command_list->impl._commandList->SetDescriptorHeaps(1, &descriptorHeap);
	command_list->impl._commandList->SetComputeRootDescriptorTable(0, output->impl._texture_handle);
	command_list->impl._commandList->SetComputeRootShaderResourceView(1, accel->impl.top_level_accel->GetGPUVirtualAddress());
	auto cbGpuAddress = pipeline->_constant_buffer->impl._buffer->GetGPUVirtualAddress();
	command_list->impl._commandList->SetComputeRootConstantBufferView(2, cbGpuAddress);

	// Since each shader table has only one shader record, the stride is same as the size.
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.HitGroupTable.StartAddress = pipeline->impl.hitgroup_shader_table->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = pipeline->impl.hitgroup_shader_table->GetDesc().Width;
	dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
	dispatchDesc.MissShaderTable.StartAddress = pipeline->impl.miss_shader_table->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = pipeline->impl.miss_shader_table->GetDesc().Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline->impl.raygen_shader_table->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = pipeline->impl.raygen_shader_table->GetDesc().Width;
	dispatchDesc.Width = output->_width;
	dispatchDesc.Height = output->_height;
	dispatchDesc.Depth = 1;
	dxrCommandList->SetPipelineState1(pipeline->impl.dxr_state);
	dxrCommandList->DispatchRays(&dispatchDesc);
}

void kinc_raytrace_copy_target(kinc_g5_command_list_t *command_list, kinc_g5_render_target_t* render_target, kinc_raytrace_target_t *output) {
	D3D12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(render_target->impl.renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(output->impl._texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	command_list->impl._commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	command_list->impl._commandList->CopyResource(render_target->impl.renderTarget, output->impl._texture);

	D3D12_RESOURCE_BARRIER postCopyBarriers[2];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(render_target->impl.renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(output->impl._texture, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	command_list->impl._commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

#endif // KORE_DXR
