#include "commandlist_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

#include "pipeline_structs.h"

#include <assert.h>

void kope_d3d12_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	list->d3d12.compute_pipeline_set = false;

	kope_g5_texture *render_target = parameters->color_attachments[0].texture;

	if (render_target->d3d12.in_flight_frame_index > 0) {
		list->d3d12.blocking_frame_index = render_target->d3d12.in_flight_frame_index;
	}

	if (render_target->d3d12.resource_state != D3D12_RESOURCE_STATE_RENDER_TARGET) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)render_target->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		render_target->d3d12.resource_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = list->d3d12.device->all_rtvs->GetCPUDescriptorHandleForHeapStart();
	rtv.ptr += render_target->d3d12.rtv_index * list->d3d12.device->rtv_increment;

	D3D12_RECT scissor = {0, 0, 1024, 768};

	D3D12_VIEWPORT viewport = {0.0f, 0.0f, 1024.0f, 768.0f, 0.0f, 1.0f};

	list->d3d12.list->OMSetRenderTargets(1, &rtv, true, nullptr);
	list->d3d12.list->RSSetViewports(1, &viewport);
	list->d3d12.list->RSSetScissorRects(1, &scissor);

	FLOAT color[4];
	memcpy(color, &parameters->color_attachments[0].clear_value, sizeof(color));
	list->d3d12.list->ClearRenderTargetView(rtv, color, 0, NULL);
}

void kope_d3d12_command_list_end_render_pass(kope_g5_command_list *list) {}

void kope_d3d12_command_list_present(kope_g5_command_list *list) {
	list->d3d12.presenting = true;
}

void kope_d3d12_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                              uint64_t size) {
	D3D12_GPU_VIRTUAL_ADDRESS address = buffer->d3d12.resource->GetGPUVirtualAddress();
	address += offset;

	D3D12_INDEX_BUFFER_VIEW view;
	view.BufferLocation = address;
	assert(size <= UINT32_MAX);
	view.SizeInBytes = (UINT)size;
	view.Format = index_format == KOPE_G5_INDEX_FORMAT_UINT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	list->d3d12.list->IASetIndexBuffer(&view);
}

void kope_d3d12_command_list_set_vertex_buffer(kope_g5_command_list *list, uint32_t slot, kope_d3d12_buffer *buffer, uint64_t offset, uint64_t size,
                                               uint64_t stride) {
	D3D12_VERTEX_BUFFER_VIEW view = {0};

	view.BufferLocation = buffer->resource->GetGPUVirtualAddress() + offset;
	view.SizeInBytes = (UINT)size;
	view.StrideInBytes = (UINT)stride;

	list->d3d12.list->IASetVertexBuffers(slot, 1, &view);
}

void kope_d3d12_command_list_set_render_pipeline(kope_g5_command_list *list, kope_d3d12_render_pipeline *pipeline) {
	list->d3d12.list->SetPipelineState(pipeline->pipe);
	list->d3d12.list->SetGraphicsRootSignature(pipeline->root_signature);
}

void kope_d3d12_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                          uint32_t first_instance) {
	list->d3d12.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->d3d12.list->DrawIndexedInstanced(index_count, instance_count, first_index, base_vertex, first_instance);
}

void kope_d3d12_command_list_set_descriptor_table(kope_g5_command_list *list, uint32_t table_index, kope_d3d12_descriptor_set *set) {
	if (set->descriptor_count > 0) {
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor = list->d3d12.device->descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		gpu_descriptor.ptr += set->descriptor_allocation.offset * list->d3d12.device->cbv_srv_uav_increment;
		if (list->d3d12.compute_pipeline_set) {
			list->d3d12.list->SetComputeRootDescriptorTable(table_index, gpu_descriptor);
		}
		else {
			list->d3d12.list->SetGraphicsRootDescriptorTable(table_index, gpu_descriptor);
		}
		table_index += 1;
	}

	if (set->sampler_count > 0) {
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor = list->d3d12.device->sampler_heap->GetGPUDescriptorHandleForHeapStart();
		gpu_descriptor.ptr += set->sampler_allocation.offset * list->d3d12.device->sampler_increment;
		if (list->d3d12.compute_pipeline_set) {
			list->d3d12.list->SetComputeRootDescriptorTable(table_index, gpu_descriptor);
		}
		else {
			list->d3d12.list->SetGraphicsRootDescriptorTable(table_index, gpu_descriptor);
		}
	}
}

void kope_d3d12_command_list_copy_buffer_to_texture(kope_g5_command_list *list, kope_g5_buffer *source, kope_g5_texture *destination, kope_uint3 size) {
	if (source->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_SOURCE) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = source->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)source->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		source->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_SOURCE;
	}

	if (destination->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_DEST) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = destination->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)destination->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		destination->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = destination->d3d12.resource;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = source->d3d12.resource;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Depth = 1;
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	src.PlacedFootprint.Footprint.Height = 512;
	src.PlacedFootprint.Footprint.RowPitch = 512 * 4;
	src.PlacedFootprint.Footprint.Width = 512;

	list->d3d12.list->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
}

void kope_d3d12_command_list_copy_texture_to_texture(kope_g5_command_list *list, kope_g5_texture *source, kope_g5_texture *destination, kope_uint3 size) {
	if (source->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_SOURCE) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = source->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)source->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		source->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_SOURCE;
	}

	if (destination->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_DEST) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = destination->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)destination->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		destination->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = destination->d3d12.resource;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = source->d3d12.resource;
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	list->d3d12.list->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
}

void kope_d3d12_command_list_set_compute_pipeline(kope_g5_command_list *list, kope_d3d12_compute_pipeline *pipeline) {
	list->d3d12.compute_pipeline_set = true;
	list->d3d12.list->SetPipelineState(pipeline->pipe);
	list->d3d12.list->SetComputeRootSignature(pipeline->root_signature);
}

void kope_d3d12_command_list_compute(kope_g5_command_list *list, uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z) {
	list->d3d12.list->Dispatch(workgroup_count_x, workgroup_count_y, workgroup_count_z);
}

void kope_d3d12_command_list_prepare_raytracing_volume(kope_g5_command_list *list, kope_g5_raytracing_volume *volume) {
	D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc = {};

	geometry_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometry_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	geometry_desc.Triangles.Transform3x4 = 0;

	geometry_desc.Triangles.IndexFormat = volume->d3d12.index_buffer != nullptr ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_UNKNOWN;
	geometry_desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometry_desc.Triangles.IndexCount = volume->d3d12.index_count;
	geometry_desc.Triangles.VertexCount = (UINT)volume->d3d12.vertex_count;
	geometry_desc.Triangles.IndexBuffer = volume->d3d12.index_buffer != nullptr ? volume->d3d12.index_buffer->d3d12.resource->GetGPUVirtualAddress() : 0;
	geometry_desc.Triangles.VertexBuffer.StartAddress = volume->d3d12.vertex_buffer->d3d12.resource->GetGPUVirtualAddress();
	geometry_desc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = 1;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.pGeometryDescs = &geometry_desc;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.DestAccelerationStructureData = volume->d3d12.acceleration_structure.d3d12.resource->GetGPUVirtualAddress();
	build_desc.Inputs = inputs;
	build_desc.ScratchAccelerationStructureData = volume->d3d12.scratch_buffer.d3d12.resource->GetGPUVirtualAddress();

	list->d3d12.list->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);
}

void kope_d3d12_command_list_prepare_raytracing_hierarchy(kope_g5_command_list *list, kope_g5_raytracing_hierarchy *hierarchy) {
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = hierarchy->d3d12.volumes_count;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = hierarchy->d3d12.instances.d3d12.resource->GetGPUVirtualAddress();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.DestAccelerationStructureData = hierarchy->d3d12.acceleration_structure.d3d12.resource->GetGPUVirtualAddress();
	build_desc.Inputs = inputs;
	build_desc.ScratchAccelerationStructureData = hierarchy->d3d12.scratch_buffer.d3d12.resource->GetGPUVirtualAddress();

	list->d3d12.list->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);
}

void kope_d3d12_command_list_set_ray_pipeline(kope_g5_command_list *list, kope_d3d12_ray_pipeline *pipeline) {
	list->d3d12.list->SetPipelineState1(pipeline->pipe);
	list->d3d12.list->SetComputeRootSignature(pipeline->root_signature);
	list->d3d12.ray_pipe = pipeline;
	list->d3d12.compute_pipeline_set = true;
}

void kope_d3d12_command_list_trace_rays(kope_g5_command_list *list) {
	D3D12_DISPATCH_RAYS_DESC desc = {};
	desc.RayGenerationShaderRecord.StartAddress = list->d3d12.ray_pipe->shader_ids.d3d12.resource->GetGPUVirtualAddress();
	desc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	desc.MissShaderTable.StartAddress = list->d3d12.ray_pipe->shader_ids.d3d12.resource->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	desc.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	desc.HitGroupTable.StartAddress =
	    list->d3d12.ray_pipe->shader_ids.d3d12.resource->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	desc.HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	desc.Width = 1024;
	desc.Height = 768;
	desc.Depth = 1;

	list->d3d12.list->DispatchRays(&desc);
}
