#include "device_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/device.h>

#include <kinc/backend/SystemMicrosoft.h>

#include <assert.h>

void kope_d3d12_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist) {
#ifndef NDEBUG
	ID3D12Debug *debug = NULL;
	if (D3D12GetDebugInterface(IID_PPV_ARGS(&debug)) == S_OK) {
		debug->EnableDebugLayer();
	}
#endif
	kinc_microsoft_affirm(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device->d3d12.device)));
}

void kope_d3d12_device_destroy(kope_g5_device *device) {
	device->d3d12.device->Release();
}

void kope_d3d12_device_set_name(kope_g5_device *device, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	device->d3d12.device->SetName(wstr);
}

void kope_d3d12_device_create_buffer(kope_g5_device *device, kope_g5_buffer_parameters parameters, kope_g5_buffer *buffer) {
	// assert(parameters.usage_flags & KOPE_G5_BUFFER_USAGE_INDEX);

	// buffer->impl.count = count;
	// buffer->impl.gpu_memory = gpuMemory;
	// buffer->impl.format = format;

	// static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	// int uploadBufferSize = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * count : sizeof(uint32_t) * count;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;
	if ((parameters.usage_flags & KOPE_G5_BUFFER_USAGE_CPU_READ) || (parameters.usage_flags & KOPE_G5_BUFFER_USAGE_CPU_WRITE)) {
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	}
	else {
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	}

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = parameters.size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	kinc_microsoft_affirm(device->d3d12.device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                                    NULL, IID_GRAPHICS_PPV_ARGS(&buffer->d3d12.buffer)));

	// buffer->impl.index_buffer_view.BufferLocation = buffer->impl.upload_buffer->GetGPUVirtualAddress();
	// buffer->impl.index_buffer_view.SizeInBytes = uploadBufferSize;
	// buffer->impl.index_buffer_view.Format = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	// buffer->impl.last_start = 0;
	// buffer->impl.last_count = kinc_g5_index_buffer_count(buffer);
}
