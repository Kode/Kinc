#include <kinc/backend/graphics5/raytrace.h>

#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/raytrace.h>
#include <kinc/graphics5/vertexbuffer.h>

static const wchar_t *hit_group_name = L"hitgroup";
static const wchar_t *raygen_shader_name = L"raygeneration";
static const wchar_t *closesthit_shader_name = L"closesthit";
static const wchar_t *miss_shader_name = L"miss";

static ID3D12Device5 *dxrDevice;
static ID3D12GraphicsCommandList4 *dxrCommandList;
static ID3D12RootSignature *dxrRootSignature;
static ID3D12DescriptorHeap *descriptorHeap;
static kinc_raytrace_acceleration_structure_t *accel;
static kinc_raytrace_pipeline_t *pipeline;
static kinc_g5_texture_t *output = NULL;

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 kinc_g5_constant_buffer_t *constant_buffer) {
	pipeline->_constant_buffer = constant_buffer;
	// Descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {0};
	// Allocate a heap for 3 descriptors:
	// 2 - bottom and top level acceleration structure
	// 1 - raytracing output texture SRV
	descriptorHeapDesc.NumDescriptors = 3;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &descriptorHeap);

	// Device
#ifndef KORE_CONSOLE
	device->lpVtbl->QueryInterface(device, &IID_ID3D12Device5, &dxrDevice);
	command_list->impl._commandList->lpVtbl->QueryInterface(command_list->impl._commandList, &IID_ID3D12GraphicsCommandList4, &dxrCommandList);
#endif

	// Root signatures
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	D3D12_DESCRIPTOR_RANGE UAVDescriptor = {0};
	UAVDescriptor.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	UAVDescriptor.NumDescriptors = 1;
	UAVDescriptor.BaseShaderRegister = 0;
	UAVDescriptor.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {0};
	// Output view
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &UAVDescriptor;
	// Acceleration structure
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	// Constant buffer
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].Descriptor.ShaderRegister = 0;

	D3D12_ROOT_SIGNATURE_DESC dxrRootSignatureDesc = {0};
	dxrRootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
	dxrRootSignatureDesc.pParameters = rootParameters;
	ID3DBlob *blob = NULL;
	ID3DBlob *error = NULL;
	D3D12SerializeRootSignature(&dxrRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	device->lpVtbl->CreateRootSignature(device, 1, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), &IID_ID3D12RootSignature,
	                                    &dxrRootSignature);

	// Pipeline
	D3D12_STATE_OBJECT_DESC raytracingPipeline = {0};
	raytracingPipeline.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	D3D12_SHADER_BYTECODE shaderBytecode = {0};
	shaderBytecode.pShaderBytecode = ray_shader;
	shaderBytecode.BytecodeLength = ray_shader_size;

	D3D12_DXIL_LIBRARY_DESC dxilLibrary = {0};
	dxilLibrary.DXILLibrary = shaderBytecode;
	D3D12_EXPORT_DESC exports[3] = {0};
	exports[0].Name = raygen_shader_name;
	exports[1].Name = closesthit_shader_name;
	exports[2].Name = miss_shader_name;
	dxilLibrary.pExports = exports;
	dxilLibrary.NumExports = 3;

	D3D12_HIT_GROUP_DESC hitGroup = {0};
	hitGroup.ClosestHitShaderImport = closesthit_shader_name;
	hitGroup.HitGroupExport = hit_group_name;
	hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {0};
	shaderConfig.MaxPayloadSizeInBytes = 4 * sizeof(float);   // float4 color
	shaderConfig.MaxAttributeSizeInBytes = 2 * sizeof(float); // float2 barycentrics

	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {0};
	pipelineConfig.MaxTraceRecursionDepth = 1; // ~ primary rays only

	D3D12_STATE_SUBOBJECT subobjects[5] = {0};
	subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subobjects[0].pDesc = &dxilLibrary;
	subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subobjects[1].pDesc = &hitGroup;
	subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	subobjects[2].pDesc = &shaderConfig;
	subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	subobjects[3].pDesc = &dxrRootSignature;
	subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	subobjects[4].pDesc = &pipelineConfig;
	raytracingPipeline.NumSubobjects = 5;
	raytracingPipeline.pSubobjects = subobjects;

#ifndef KORE_CONSOLE
	dxrDevice->lpVtbl->CreateStateObject(dxrDevice, &raytracingPipeline, &IID_ID3D12StateObject, &pipeline->impl.dxr_state);
#endif

	// Shader tables
	// Get shader identifiers
	ID3D12StateObjectProperties *stateObjectProps = NULL;
#ifndef KORE_CONSOLE
	pipeline->impl.dxr_state->lpVtbl->QueryInterface(pipeline->impl.dxr_state, &IID_ID3D12StateObjectProperties, &stateObjectProps);
#endif
	void *rayGenShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, raygen_shader_name);
	void *missShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, miss_shader_name);
	void *hitGroupShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, hit_group_name);
	UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	int align = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	// Ray gen shader table
	{
		UINT size = shaderIdSize + constant_buffer->impl.mySize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {0};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                        &IID_ID3D12Resource, &pipeline->impl.raygen_shader_table);

		D3D12_RANGE rstRange = {0};
		rstRange.Begin = 0;
		rstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.raygen_shader_table->lpVtbl->Map(pipeline->impl.raygen_shader_table, 0, &rstRange, (void **)(&byteDest));

		D3D12_RANGE cbRange = {0};
		cbRange.Begin = 0;
		cbRange.End = constant_buffer->impl.mySize;
		void *constantBufferData;
		constant_buffer->impl.constant_buffer->lpVtbl->Map(constant_buffer->impl.constant_buffer, 0, &cbRange, (void **)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->impl.mySize);
		pipeline->impl.raygen_shader_table->lpVtbl->Unmap(pipeline->impl.raygen_shader_table, 0, NULL);
	}

	// Miss shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {0};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                        &IID_ID3D12Resource, &pipeline->impl.miss_shader_table);

		D3D12_RANGE mstRange = {0};
		mstRange.Begin = 0;
		mstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.miss_shader_table->lpVtbl->Map(pipeline->impl.miss_shader_table, 0, &mstRange, (void **)(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->lpVtbl->Unmap(pipeline->impl.miss_shader_table, 0, NULL);
	}

	// Hit group shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {0};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                        &IID_ID3D12Resource, &pipeline->impl.hitgroup_shader_table);

		D3D12_RANGE hstRange = {0};
		hstRange.Begin = 0;
		hstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.hitgroup_shader_table->lpVtbl->Map(pipeline->impl.hitgroup_shader_table, 0, &hstRange, (void **)(&byteDest));
		memcpy(byteDest, hitGroupShaderId, size);
		pipeline->impl.hitgroup_shader_table->lpVtbl->Unmap(pipeline->impl.hitgroup_shader_table, 0, NULL);
	}
}

void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline) {
	pipeline->impl.dxr_state->lpVtbl->Release(pipeline->impl.dxr_state);
	pipeline->impl.raygen_shader_table->lpVtbl->Release(pipeline->impl.raygen_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->Release(pipeline->impl.miss_shader_table);
	pipeline->impl.hitgroup_shader_table->lpVtbl->Release(pipeline->impl.hitgroup_shader_table);
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list, kinc_g5_vertex_buffer_t *vb,
                                               kinc_g5_index_buffer_t *ib) {
	// Reset the command list for the acceleration structure construction
	command_list->impl._commandList->lpVtbl->Reset(command_list->impl._commandList, command_list->impl._commandAllocator, NULL);

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {0};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = ib->impl.upload_buffer->lpVtbl->GetGPUVirtualAddress(ib->impl.upload_buffer);
	geometryDesc.Triangles.IndexCount = ib->impl.count;
	geometryDesc.Triangles.IndexFormat = ib->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = vb->impl.myCount;
	geometryDesc.Triangles.VertexBuffer.StartAddress = vb->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(vb->impl.uploadBuffer);
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = D3D12ResourceGetDesc(vb->impl.uploadBuffer).Width / vb->impl.myCount;
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// Get required sizes for an acceleration structure
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {0};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {0};
	dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice, &topLevelInputs, &topLevelPrebuildInfo);

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {0};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;
	bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice, &bottomLevelInputs, &bottomLevelPrebuildInfo);

	ID3D12Resource *scratchResource;
	{
		UINT64 tlSize = topLevelPrebuildInfo.ScratchDataSizeInBytes;
		UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
		D3D12_RESOURCE_DESC bufferDesc = {0};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = tlSize > blSize ? tlSize : blSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL,
		                                        &IID_ID3D12Resource, &scratchResource);
	}

	// Allocate resources for acceleration structures
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS.
	{
		D3D12_RESOURCE_DESC bufferDesc = {0};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		                                        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &IID_ID3D12Resource,
		                                        &accel->impl.bottom_level_accel);
	}
	{
		D3D12_RESOURCE_DESC bufferDesc = {0};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		                                        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &IID_ID3D12Resource,
		                                        &accel->impl.top_level_accel);
	}

	// Create an instance desc for the bottom-level acceleration structure
	ID3D12Resource *instanceDescs;
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {0};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = accel->impl.bottom_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel);

	D3D12_RESOURCE_DESC bufferDesc = {0};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = sizeof(instanceDesc);
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {0};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProperties.CreationNodeMask = 1;
	uploadHeapProperties.VisibleNodeMask = 1;

	device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                        &IID_ID3D12Resource, &instanceDescs);
	void *mappedData;
	instanceDescs->lpVtbl->Map(instanceDescs, 0, NULL, &mappedData);
	memcpy(mappedData, &instanceDesc, sizeof(instanceDesc));
	instanceDescs->lpVtbl->Unmap(instanceDescs, 0, NULL);

	// Bottom Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {0};
	bottomLevelBuildDesc.Inputs = bottomLevelInputs;
	bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);
	bottomLevelBuildDesc.DestAccelerationStructureData = accel->impl.bottom_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel);

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = bottomLevelBuildDesc;
	topLevelInputs.InstanceDescs = instanceDescs->lpVtbl->GetGPUVirtualAddress(instanceDescs);
	topLevelBuildDesc.Inputs = topLevelInputs;
	topLevelBuildDesc.DestAccelerationStructureData = accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel);
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);

	// Build acceleration structure
	dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &bottomLevelBuildDesc, 0, NULL);
	D3D12_RESOURCE_BARRIER barrier = {0};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = accel->impl.bottom_level_accel;
	command_list->impl._commandList->lpVtbl->ResourceBarrier(command_list->impl._commandList, 1, &barrier);
	dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &topLevelBuildDesc, 0, NULL);

	kinc_g5_command_list_execute(command_list);
	kinc_g5_command_list_wait_for_execution_to_finish(command_list);
}

void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel) {
	accel->impl.bottom_level_accel->lpVtbl->Release(accel->impl.bottom_level_accel);
	accel->impl.top_level_accel->lpVtbl->Release(accel->impl.top_level_accel);
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_g5_texture_t *_output) {
	if (_output != output) {
		output = _output;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {0};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		device->lpVtbl->CreateUnorderedAccessView(device, output->impl.image, NULL, &uavDesc, GetCPUDescriptorHandle(descriptorHeap));
	}
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	command_list->impl._commandList->lpVtbl->SetComputeRootSignature(command_list->impl._commandList, dxrRootSignature);

	// Bind the heaps, acceleration structure and dispatch rays
	command_list->impl._commandList->lpVtbl->SetDescriptorHeaps(command_list->impl._commandList, 1, &descriptorHeap);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 0, GetGPUDescriptorHandle(descriptorHeap));
	command_list->impl._commandList->lpVtbl->SetComputeRootShaderResourceView(
	    command_list->impl._commandList, 1, accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel));
	D3D12_GPU_VIRTUAL_ADDRESS cbGpuAddress =
	    pipeline->_constant_buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(pipeline->_constant_buffer->impl.constant_buffer);
	command_list->impl._commandList->lpVtbl->SetComputeRootConstantBufferView(command_list->impl._commandList, 2, cbGpuAddress);

	// Since each shader table has only one shader record, the stride is same as the size.
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {0};
	dispatchDesc.HitGroupTable.StartAddress = pipeline->impl.hitgroup_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.hitgroup_shader_table);
	dispatchDesc.HitGroupTable.SizeInBytes = D3D12ResourceGetDesc(pipeline->impl.hitgroup_shader_table).Width;
	dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
	dispatchDesc.MissShaderTable.StartAddress = pipeline->impl.miss_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.miss_shader_table);
	dispatchDesc.MissShaderTable.SizeInBytes = D3D12ResourceGetDesc(pipeline->impl.miss_shader_table).Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline->impl.raygen_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.raygen_shader_table);
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = D3D12ResourceGetDesc(pipeline->impl.raygen_shader_table).Width;
	dispatchDesc.Width = output->texWidth;
	dispatchDesc.Height = output->texHeight;
	dispatchDesc.Depth = 1;
	dxrCommandList->lpVtbl->SetPipelineState1(dxrCommandList, pipeline->impl.dxr_state);
	dxrCommandList->lpVtbl->DispatchRays(dxrCommandList, &dispatchDesc);
}

void kinc_raytrace_copy(kinc_g5_command_list_t *command_list, kinc_g5_render_target_t *target, kinc_g5_texture_t *source) {
	D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {0};
	preCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	preCopyBarriers[0].Transition.pResource = target->impl.renderTarget;
	preCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	preCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	preCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	preCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	preCopyBarriers[1].Transition.pResource = source->impl.image;
	preCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	preCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	preCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list->impl._commandList->lpVtbl->ResourceBarrier(command_list->impl._commandList, ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	command_list->impl._commandList->lpVtbl->CopyResource(command_list->impl._commandList, target->impl.renderTarget, source->impl.image);

	D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {0};
	postCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	postCopyBarriers[0].Transition.pResource = target->impl.renderTarget;
	postCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	postCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	postCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	postCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	postCopyBarriers[1].Transition.pResource = source->impl.image;
	postCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	postCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	postCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list->impl._commandList->lpVtbl->ResourceBarrier(command_list->impl._commandList, ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}
