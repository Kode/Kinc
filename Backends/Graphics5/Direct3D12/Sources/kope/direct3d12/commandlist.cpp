#include "commandlist_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

#include "pipeline_structs.h"

#include <assert.h>

void kope_d3d12_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
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

void kope_d3d12_command_list_set_pipeline(kope_g5_command_list *list, kope_d3d12_pipeline *pipeline) {
	list->d3d12.list->SetPipelineState(pipeline->pipe);
	list->d3d12.list->SetGraphicsRootSignature(pipeline->root_signature);
}

void kope_d3d12_command_list_draw_indexed(kope_g5_command_list *list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t base_vertex,
                                          uint32_t first_instance) {
	list->d3d12.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->d3d12.list->DrawIndexedInstanced(index_count, instance_count, first_index, base_vertex, first_instance);
}

void kope_d3d12_command_list_set_descriptor_table(kope_g5_command_list *list, uint32_t table_index, kope_d3d12_descriptor_set *set) {
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor = list->d3d12.device->descriptor_heap->GetGPUDescriptorHandleForHeapStart();
	gpu_descriptor.ptr += set->allocation.offset * list->d3d12.device->cbv_srv_uav_increment;
	list->d3d12.list->SetGraphicsRootDescriptorTable(table_index, gpu_descriptor);
}
