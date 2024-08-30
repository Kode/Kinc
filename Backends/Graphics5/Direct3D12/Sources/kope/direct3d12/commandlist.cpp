#include "commandlist_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>

void kope_d3d12_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	list->d3d12.render_pass_framebuffer = NULL;

	kope_g5_texture *framebuffer = parameters->color_attachments[0].texture;

	if (framebuffer->d3d12.resource_state != D3D12_RESOURCE_STATE_RENDER_TARGET) {
		if (framebuffer->d3d12.resource_state == D3D12_RESOURCE_STATE_PRESENT) {
			list->d3d12.render_pass_framebuffer = &framebuffer->d3d12;
		}

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = framebuffer->d3d12.resource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)framebuffer->d3d12.resource_state;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->d3d12.list->ResourceBarrier(1, &barrier);

		framebuffer->d3d12.resource_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = list->d3d12.device->all_rtvs->GetCPUDescriptorHandleForHeapStart();
	rtv.ptr += framebuffer->d3d12.rtv_index * list->d3d12.device->rtv_increment;

	FLOAT color[4];
	memcpy(color, &parameters->color_attachments[0].clear_value, sizeof(color));
	list->d3d12.list->ClearRenderTargetView(rtv, color, 0, NULL);
}

void kope_d3d12_command_list_end_render_pass(kope_g5_command_list *list) {
	if (list->d3d12.render_pass_framebuffer != NULL) {
		kope_d3d12_texture *framebuffer = list->d3d12.render_pass_framebuffer;

		if (framebuffer->resource_state != D3D12_RESOURCE_STATE_PRESENT) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = framebuffer->resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)framebuffer->resource_state;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			list->d3d12.list->ResourceBarrier(1, &barrier);

			framebuffer->resource_state = D3D12_RESOURCE_STATE_PRESENT;
		}
	}
}

void kope_d3d12_command_list_finish(kope_g5_command_list *list) {
	list->d3d12.list->Close();
}
