#include "commandlist_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

#include <kope/direct3d12/texture_functions.h>

#include <kinc/backend/SystemMicrosoft.h>

#include "pipeline_structs.h"

#include <assert.h>

#ifdef KOPE_PIX
#include <WinPixEventRuntime/pix3.h>
#endif

void kope_d3d12_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	list->d3d12.compute_pipeline_set = false;

	D3D12_CPU_DESCRIPTOR_HANDLE render_target_views = list->d3d12.rtv_descriptors->GetCPUDescriptorHandleForHeapStart();

	for (size_t render_target_index = 0; render_target_index < parameters->color_attachments_count; ++render_target_index) {
		kope_g5_texture *render_target = parameters->color_attachments[render_target_index].texture;

		if (render_target->d3d12.in_flight_frame_index > 0) {
			list->d3d12.blocking_frame_index = render_target->d3d12.in_flight_frame_index;
		}

		if (render_target->d3d12
		        .resource_states[kope_d3d12_texture_resource_state_index(render_target, 0, parameters->color_attachments[render_target_index].depth_slice)] !=
		    D3D12_RESOURCE_STATE_RENDER_TARGET) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = render_target->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore =
			    (D3D12_RESOURCE_STATES)render_target->d3d12
			        .resource_states[kope_d3d12_texture_resource_state_index(render_target, 0, parameters->color_attachments[render_target_index].depth_slice)];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12CalcSubresource(0, parameters->color_attachments[render_target_index].depth_slice, 0,
			                                                      render_target->d3d12.mip_level_count, render_target->d3d12.depth_or_array_layers);

			list->d3d12.list->ResourceBarrier(1, &barrier);

			render_target->d3d12
			    .resource_states[kope_d3d12_texture_resource_state_index(render_target, 0, parameters->color_attachments[render_target_index].depth_slice)] =
			    D3D12_RESOURCE_STATE_RENDER_TARGET;
		}

		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		if (parameters->color_attachments[render_target_index].depth_slice != 0) {
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = parameters->color_attachments[render_target_index].depth_slice;
			desc.Texture2DArray.ArraySize = 1;
		}
		else {
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE render_target_view = render_target_views;
		render_target_view.ptr += render_target_index * list->d3d12.rtv_increment;

		list->d3d12.device->device->CreateRenderTargetView(render_target->d3d12.resource, &desc, render_target_view);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view = list->d3d12.dsv_descriptor->GetCPUDescriptorHandleForHeapStart();

	if (parameters->depth_stencil_attachment.texture != NULL) {
		kope_g5_texture *render_target = parameters->depth_stencil_attachment.texture;

		if (render_target->d3d12.in_flight_frame_index > 0) {
			list->d3d12.blocking_frame_index = render_target->d3d12.in_flight_frame_index;
		}

		if (render_target->d3d12.resource_states[0] != D3D12_RESOURCE_STATE_DEPTH_WRITE) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = render_target->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)render_target->d3d12.resource_states[0];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			barrier.Transition.Subresource = 0;

			list->d3d12.list->ResourceBarrier(1, &barrier);

			render_target->d3d12.resource_states[0] = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = (DXGI_FORMAT)render_target->d3d12.format;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		list->d3d12.device->device->CreateDepthStencilView(render_target->d3d12.resource, &desc, depth_stencil_view);
	}

	if (parameters->color_attachments_count > 0) {
		if (parameters->depth_stencil_attachment.texture != NULL) {
			list->d3d12.list->OMSetRenderTargets((UINT)parameters->color_attachments_count, &render_target_views, TRUE, &depth_stencil_view);
		}
		else {
			list->d3d12.list->OMSetRenderTargets((UINT)parameters->color_attachments_count, &render_target_views, TRUE, nullptr);
		}
	}
	else if (parameters->depth_stencil_attachment.texture != NULL) {
		list->d3d12.list->OMSetRenderTargets(0, NULL, TRUE, &depth_stencil_view);
	}

	if (parameters->color_attachments_count > 0) {
		D3D12_VIEWPORT viewport = {
		    0.0f, 0.0f, (FLOAT)parameters->color_attachments[0].texture->d3d12.width, (FLOAT)parameters->color_attachments[0].texture->d3d12.height,
		    0.0f, 1.0f};
		list->d3d12.list->RSSetViewports(1, &viewport);

		D3D12_RECT scissor = {0, 0, (LONG)parameters->color_attachments[0].texture->d3d12.width, (LONG)parameters->color_attachments[0].texture->d3d12.height};
		list->d3d12.list->RSSetScissorRects(1, &scissor);
	}
	else if (parameters->depth_stencil_attachment.texture != NULL) {
		D3D12_VIEWPORT viewport = {
		    0.0f, 0.0f, (FLOAT)parameters->depth_stencil_attachment.texture->d3d12.width, (FLOAT)parameters->depth_stencil_attachment.texture->d3d12.height,
		    0.0f, 1.0f};
		list->d3d12.list->RSSetViewports(1, &viewport);

		D3D12_RECT scissor = {0, 0, (LONG)parameters->depth_stencil_attachment.texture->d3d12.width,
		                      (LONG)parameters->depth_stencil_attachment.texture->d3d12.height};
		list->d3d12.list->RSSetScissorRects(1, &scissor);
	}

	for (size_t render_target_index = 0; render_target_index < parameters->color_attachments_count; ++render_target_index) {
		if (parameters->color_attachments[render_target_index].load_op == KOPE_G5_LOAD_OP_CLEAR) {
			D3D12_CPU_DESCRIPTOR_HANDLE render_target_view = render_target_views;
			render_target_view.ptr += render_target_index * list->d3d12.rtv_increment;

			FLOAT color[4];
			memcpy(color, &parameters->color_attachments[render_target_index].clear_value, sizeof(color));
			list->d3d12.list->ClearRenderTargetView(render_target_view, color, 0, NULL);
		}
	}

	if (parameters->depth_stencil_attachment.texture != NULL) {
		if (parameters->depth_stencil_attachment.depth_load_op == KOPE_G5_LOAD_OP_CLEAR) {
			list->d3d12.list->ClearDepthStencilView(depth_stencil_view, D3D12_CLEAR_FLAG_DEPTH, parameters->depth_stencil_attachment.depth_clear_value, 0, 0,
			                                        NULL);
		}
	}

	list->d3d12.occlusion_query_set = parameters->occlusion_query_set;

	list->d3d12.timestamp_query_set = parameters->timestamp_writes.query_set;
	list->d3d12.timestamp_beginning_of_pass_write_index = parameters->timestamp_writes.beginning_of_pass_write_index;
	list->d3d12.timestamp_end_of_pass_write_index = parameters->timestamp_writes.end_of_pass_write_index;

	if (list->d3d12.timestamp_query_set != NULL) {
		list->d3d12.list->EndQuery(list->d3d12.timestamp_query_set->d3d12.query_heap, D3D12_QUERY_TYPE_TIMESTAMP,
		                           list->d3d12.timestamp_beginning_of_pass_write_index);
	}
}

void kope_d3d12_command_list_end_render_pass(kope_g5_command_list *list) {
	if (list->d3d12.timestamp_query_set != NULL) {
		list->d3d12.list->EndQuery(list->d3d12.timestamp_query_set->d3d12.query_heap, D3D12_QUERY_TYPE_TIMESTAMP,
		                           list->d3d12.timestamp_end_of_pass_write_index);
	}
}

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

void kope_d3d12_command_list_draw(kope_g5_command_list *list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
	list->d3d12.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->d3d12.list->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
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

void kope_d3d12_command_list_set_root_constants(kope_g5_command_list *list, uint32_t table_index, const void *data, size_t data_size) {
	if (list->d3d12.compute_pipeline_set) {
		list->d3d12.list->SetComputeRoot32BitConstants(table_index, (UINT)(data_size / 4), data, 0);
	}
	else {
		list->d3d12.list->SetGraphicsRoot32BitConstants(table_index, (UINT)(data_size / 4), data, 0);
	}
}

void kope_d3d12_command_list_copy_buffer_to_buffer(kope_g5_command_list *list, kope_g5_buffer *source, uint64_t source_offset, kope_g5_buffer *destination,
                                                   uint64_t destination_offset, uint64_t size) {
	if (source->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_SOURCE && source->d3d12.resource_state != D3D12_RESOURCE_STATE_GENERIC_READ) {
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

	list->d3d12.list->CopyBufferRegion(destination->d3d12.resource, destination_offset, source->d3d12.resource, source_offset, size);
}

void kope_d3d12_command_list_copy_buffer_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_buffer *source,
                                                    const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                    uint32_t depth_or_array_layers) {
	if (source->buffer->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_SOURCE && source->buffer->d3d12.resource_state != D3D12_RESOURCE_STATE_GENERIC_READ) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = source->buffer->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)source->buffer->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		source->buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_SOURCE;
	}

	for (uint32_t array_layer = destination->origin_z; array_layer < destination->origin_z + depth_or_array_layers; ++array_layer) {
		if (destination->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(destination->texture, destination->mip_level, array_layer)] !=
		    D3D12_RESOURCE_STATE_COPY_DEST) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = destination->texture->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore =
			    (D3D12_RESOURCE_STATES)destination->texture->d3d12
			        .resource_states[kope_d3d12_texture_resource_state_index(destination->texture, destination->mip_level, array_layer)];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.Subresource = D3D12CalcSubresource(destination->mip_level, destination->origin_z, 0, destination->texture->d3d12.mip_level_count,
			                                                      destination->texture->d3d12.depth_or_array_layers);

			list->d3d12.list->ResourceBarrier(1, &barrier);

			destination->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(destination->texture, destination->mip_level, array_layer)] =
			    D3D12_RESOURCE_STATE_COPY_DEST;
		}
	}

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = source->buffer->d3d12.resource;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = source->offset;
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	src.PlacedFootprint.Footprint.RowPitch = source->bytes_per_row;
	src.PlacedFootprint.Footprint.Width = width;
	src.PlacedFootprint.Footprint.Height = height;
	src.PlacedFootprint.Footprint.Depth = depth_or_array_layers;

	D3D12_BOX source_box = {0};
	source_box.left = 0;
	source_box.right = source_box.left + width;
	source_box.top = 0;
	source_box.bottom = source_box.top + height;
	source_box.front = 0;
	source_box.back = source_box.front + depth_or_array_layers;

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = destination->texture->d3d12.resource;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = D3D12CalcSubresource(destination->mip_level, destination->origin_z, 0, destination->texture->d3d12.mip_level_count,
	                                            destination->texture->d3d12.depth_or_array_layers);

	list->d3d12.list->CopyTextureRegion(&dst, destination->origin_x, destination->origin_y, 0, &src,
	                                    &source_box); // Set DstZ to zero because it has already been selected via dst.SubresourceIndex
}

void kope_d3d12_command_list_copy_texture_to_buffer(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                    const kope_g5_image_copy_buffer *destination, uint32_t width, uint32_t height,
                                                    uint32_t depth_or_array_layers) {
	for (uint32_t array_layer = source->origin_z; array_layer < source->origin_z + depth_or_array_layers; ++array_layer) {
		if (source->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(source->texture, source->mip_level, array_layer)] !=
		    D3D12_RESOURCE_STATE_COPY_SOURCE) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = source->texture->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore =
			    (D3D12_RESOURCE_STATES)
			        source->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(source->texture, source->mip_level, array_layer)];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.Subresource = D3D12CalcSubresource(source->mip_level, source->origin_z, 0, source->texture->d3d12.mip_level_count,
			                                                      source->texture->d3d12.depth_or_array_layers);

			list->d3d12.list->ResourceBarrier(1, &barrier);

			source->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(source->texture, source->mip_level, array_layer)] =
			    D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
	}

	if (destination->buffer->d3d12.resource_state != D3D12_RESOURCE_STATE_COPY_DEST) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = destination->buffer->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)destination->buffer->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		destination->buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = source->texture->d3d12.resource;
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex =
	    D3D12CalcSubresource(source->mip_level, source->origin_z, 0, source->texture->d3d12.mip_level_count, source->texture->d3d12.depth_or_array_layers);

	D3D12_BOX source_box = {0};
	source_box.left = 0;
	source_box.right = source_box.left + width;
	source_box.top = 0;
	source_box.bottom = source_box.top + height;
	source_box.front = 0;
	source_box.back = source_box.front + depth_or_array_layers;

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = destination->buffer->d3d12.resource;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dst.PlacedFootprint.Offset = destination->offset;
	dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dst.PlacedFootprint.Footprint.RowPitch = destination->bytes_per_row;
	dst.PlacedFootprint.Footprint.Width = width;
	dst.PlacedFootprint.Footprint.Height = height;
	dst.PlacedFootprint.Footprint.Depth = depth_or_array_layers;

	list->d3d12.list->CopyTextureRegion(&dst, 0, 0, 0, &src, &source_box);
}

void kope_d3d12_command_list_copy_texture_to_texture(kope_g5_command_list *list, const kope_g5_image_copy_texture *source,
                                                     const kope_g5_image_copy_texture *destination, uint32_t width, uint32_t height,
                                                     uint32_t depth_or_array_layers) {
	for (uint32_t array_layer = source->origin_z; array_layer < source->origin_z + depth_or_array_layers; ++array_layer) {
		if (source->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(source->texture, source->mip_level, array_layer)] !=
		    D3D12_RESOURCE_STATE_COPY_SOURCE) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = source->texture->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore =
			    (D3D12_RESOURCE_STATES)
			        source->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(source->texture, source->mip_level, array_layer)];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.Subresource = D3D12CalcSubresource(source->mip_level, source->origin_z, 0, source->texture->d3d12.mip_level_count,
			                                                      source->texture->d3d12.depth_or_array_layers);

			list->d3d12.list->ResourceBarrier(1, &barrier);

			source->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(source->texture, source->mip_level, array_layer)] =
			    D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
	}

	for (uint32_t array_layer = destination->origin_z; array_layer < destination->origin_z + depth_or_array_layers; ++array_layer) {
		if (destination->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(destination->texture, destination->mip_level, array_layer)] !=
		    D3D12_RESOURCE_STATE_COPY_DEST) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = destination->texture->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore =
			    (D3D12_RESOURCE_STATES)destination->texture->d3d12
			        .resource_states[kope_d3d12_texture_resource_state_index(destination->texture, destination->mip_level, array_layer)];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.Subresource = D3D12CalcSubresource(destination->mip_level, destination->origin_z, 0, destination->texture->d3d12.mip_level_count,
			                                                      destination->texture->d3d12.depth_or_array_layers);

			list->d3d12.list->ResourceBarrier(1, &barrier);

			destination->texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(destination->texture, destination->mip_level, array_layer)] =
			    D3D12_RESOURCE_STATE_COPY_DEST;
		}
	}

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = source->texture->d3d12.resource;
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex =
	    D3D12CalcSubresource(source->mip_level, source->origin_z, 0, source->texture->d3d12.mip_level_count, source->texture->d3d12.depth_or_array_layers);

	D3D12_BOX source_box = {0};
	source_box.left = source->origin_x;
	source_box.right = source_box.left + width;
	source_box.top = source->origin_y;
	source_box.bottom = source_box.top + height;
	source_box.front = 0;
	source_box.back = 1;

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = destination->texture->d3d12.resource;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = D3D12CalcSubresource(destination->mip_level, destination->origin_z, 0, destination->texture->d3d12.mip_level_count,
	                                            destination->texture->d3d12.depth_or_array_layers);

	list->d3d12.list->CopyTextureRegion(&dst, destination->origin_x, destination->origin_y, destination->origin_z, &src, &source_box);
}

void kope_d3d12_command_list_clear_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, size_t offset, uint64_t size) {
	UINT values[4] = {0};
	// TODO
	// list->d3d12.list->ClearUnorderedAccessViewUint(a, b, buffer->d3d12.resource, values, 0, NULL);
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

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = volume->d3d12.acceleration_structure.d3d12.resource;

	list->d3d12.list->ResourceBarrier(1, &barrier);
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

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = hierarchy->d3d12.acceleration_structure.d3d12.resource;

	list->d3d12.list->ResourceBarrier(1, &barrier);
}

void kope_d3d12_command_list_update_raytracing_hierarchy(kope_g5_command_list *list, kinc_matrix4x4_t *volume_transforms, uint32_t volumes_count,
                                                         kope_g5_raytracing_hierarchy *hierarchy) {
	D3D12_RAYTRACING_INSTANCE_DESC *descs = (D3D12_RAYTRACING_INSTANCE_DESC *)kope_g5_buffer_lock(&hierarchy->d3d12.instances);

	for (uint32_t volume_index = 0; volume_index < hierarchy->d3d12.volumes_count; ++volume_index) {
		descs[volume_index].Transform[0][0] = volume_transforms[volume_index].m[0];
		descs[volume_index].Transform[1][0] = volume_transforms[volume_index].m[1];
		descs[volume_index].Transform[2][0] = volume_transforms[volume_index].m[2];

		descs[volume_index].Transform[0][1] = volume_transforms[volume_index].m[4];
		descs[volume_index].Transform[1][1] = volume_transforms[volume_index].m[5];
		descs[volume_index].Transform[2][1] = volume_transforms[volume_index].m[6];

		descs[volume_index].Transform[0][2] = volume_transforms[volume_index].m[8];
		descs[volume_index].Transform[1][2] = volume_transforms[volume_index].m[9];
		descs[volume_index].Transform[2][2] = volume_transforms[volume_index].m[10];

		descs[volume_index].Transform[0][3] = volume_transforms[volume_index].m[12];
		descs[volume_index].Transform[1][3] = volume_transforms[volume_index].m[13];
		descs[volume_index].Transform[2][3] = volume_transforms[volume_index].m[14];
	}

	kope_g5_buffer_unlock(&hierarchy->d3d12.instances);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	inputs.NumDescs = hierarchy->d3d12.volumes_count;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = hierarchy->d3d12.instances.d3d12.resource->GetGPUVirtualAddress();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.Inputs = inputs;
	build_desc.SourceAccelerationStructureData = hierarchy->d3d12.acceleration_structure.d3d12.resource->GetGPUVirtualAddress();
	build_desc.DestAccelerationStructureData = hierarchy->d3d12.acceleration_structure.d3d12.resource->GetGPUVirtualAddress();
	build_desc.ScratchAccelerationStructureData = hierarchy->d3d12.update_scratch_buffer.d3d12.resource->GetGPUVirtualAddress();

	list->d3d12.list->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = hierarchy->d3d12.acceleration_structure.d3d12.resource;

	list->d3d12.list->ResourceBarrier(1, &barrier);
}

void kope_d3d12_command_list_set_ray_pipeline(kope_g5_command_list *list, kope_d3d12_ray_pipeline *pipeline) {
	list->d3d12.list->SetPipelineState1(pipeline->pipe);
	list->d3d12.list->SetComputeRootSignature(pipeline->root_signature);
	list->d3d12.ray_pipe = pipeline;
	list->d3d12.compute_pipeline_set = true;
}

void kope_d3d12_command_list_trace_rays(kope_g5_command_list *list, uint32_t width, uint32_t height, uint32_t depth) {
	D3D12_DISPATCH_RAYS_DESC desc = {};
	desc.RayGenerationShaderRecord.StartAddress = list->d3d12.ray_pipe->shader_ids.d3d12.resource->GetGPUVirtualAddress();
	desc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	desc.MissShaderTable.StartAddress = list->d3d12.ray_pipe->shader_ids.d3d12.resource->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	desc.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	desc.HitGroupTable.StartAddress =
	    list->d3d12.ray_pipe->shader_ids.d3d12.resource->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	desc.HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;

	list->d3d12.list->DispatchRays(&desc);
}

void kope_d3d12_command_list_set_viewport(kope_g5_command_list *list, float x, float y, float width, float height, float min_depth, float max_depth) {
	D3D12_VIEWPORT viewport = {0};
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = min_depth;
	viewport.MaxDepth = max_depth;
	list->d3d12.list->RSSetViewports(1, &viewport);
}

void kope_d3d12_command_list_set_scissor_rect(kope_g5_command_list *list, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	D3D12_RECT rect = {0};
	rect.left = x;
	rect.right = rect.left + width;
	rect.top = y;
	rect.bottom = rect.top + height;
	list->d3d12.list->RSSetScissorRects(1, &rect);
}

void kope_d3d12_command_list_set_blend_constant(kope_g5_command_list *list, kope_g5_color color) {
	float color_array[4] = {0};
	color_array[0] = color.r;
	color_array[1] = color.g;
	color_array[2] = color.b;
	color_array[3] = color.a;
	list->d3d12.list->OMSetBlendFactor(color_array);
}

void kope_d3d12_command_list_set_stencil_reference(kope_g5_command_list *list, uint32_t reference) {
	list->d3d12.list->OMSetStencilRef(reference);
}

void kope_d3d12_command_list_set_name(kope_g5_command_list *list, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	list->d3d12.list->SetName(wstr);
}

void kope_d3d12_command_list_push_debug_group(kope_g5_command_list *list, const char *name) {
#ifdef KOPE_PIX
	PIXBeginEvent(list->d3d12.list, 0, "%s", name);
#endif
}

void kope_d3d12_command_list_pop_debug_group(kope_g5_command_list *list) {
#ifdef KOPE_PIX
	PIXEndEvent(list->d3d12.list);
#endif
}

void kope_d3d12_command_list_insert_debug_marker(kope_g5_command_list *list, const char *name) {
#ifdef KOPE_PIX
	PIXSetMarker(list->d3d12.list, 0, "%s", name);
#endif
}

void kope_d3d12_command_list_begin_occlusion_query(kope_g5_command_list *list, uint32_t query_index) {
	list->d3d12.current_occlusion_query_index = query_index;
	list->d3d12.list->BeginQuery(list->d3d12.occlusion_query_set->d3d12.query_heap, D3D12_QUERY_TYPE_OCCLUSION, query_index);
}

void kope_d3d12_command_list_end_occlusion_query(kope_g5_command_list *list) {
	list->d3d12.list->EndQuery(list->d3d12.occlusion_query_set->d3d12.query_heap, D3D12_QUERY_TYPE_OCCLUSION, list->d3d12.current_occlusion_query_index);
}

void kope_d3d12_command_list_resolve_query_set(kope_g5_command_list *list, kope_g5_query_set *query_set, uint32_t first_query, uint32_t query_count,
                                               kope_g5_buffer *destination, uint64_t destination_offset) {
	list->d3d12.list->ResolveQueryData(query_set->d3d12.query_heap,
	                                   query_set->d3d12.query_type == KOPE_G5_QUERY_TYPE_OCCLUSION ? D3D12_QUERY_TYPE_OCCLUSION : D3D12_QUERY_TYPE_TIMESTAMP,
	                                   first_query, query_count, destination->d3d12.resource, destination_offset);
}
