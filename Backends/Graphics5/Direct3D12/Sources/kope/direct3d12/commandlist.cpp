#include "commandlist_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

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