#include "device_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/device.h>

#include <kinc/backend/SystemMicrosoft.h>
#include <kinc/backend/Windows.h>

#include <kinc/window.h>

#include <assert.h>

#include <dxgi1_4.h>

void kope_d3d12_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
#ifndef NDEBUG
	ID3D12Debug *debug = NULL;
	if (D3D12GetDebugInterface(IID_PPV_ARGS(&debug)) == S_OK) {
		debug->EnableDebugLayer();
	}
#endif

	kinc_microsoft_affirm(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device->d3d12.device)));

	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		kinc_microsoft_affirm(device->d3d12.device->CreateCommandQueue(&desc, IID_PPV_ARGS(&device->d3d12.queue)));
	}

	{
		DXGI_SWAP_CHAIN_DESC desc = {0};
		desc.BufferCount = 2;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferDesc.Width = kinc_window_width(0);
		desc.BufferDesc.Height = kinc_window_height(0);
		desc.OutputWindow = kinc_windows_window_handle(0);
		desc.SampleDesc.Count = 1;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Windowed = true;

		IDXGIFactory4 *dxgi_factory = NULL;
		kinc_microsoft_affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));

		kinc_microsoft_affirm(dxgi_factory->CreateSwapChain((IUnknown *)device->d3d12.queue, &desc, &device->d3d12.swap_chain));
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = KOPE_INDEX_ALLOCATOR_SIZE;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&device->d3d12.all_rtvs)));

		kope_index_allocator_init(&device->d3d12.rtv_index_allocator);

		device->d3d12.rtv_increment = device->d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = KOPE_INDEX_ALLOCATOR_SIZE;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&device->d3d12.all_dsvs)));

		kope_index_allocator_init(&device->d3d12.dsv_index_allocator);

		device->d3d12.dsv_increment = device->d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}
}

void kope_d3d12_device_destroy(kope_g5_device *device) {
	device->d3d12.device->Release();
}

void kope_d3d12_device_set_name(kope_g5_device *device, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	device->d3d12.device->SetName(wstr);
}

void kope_d3d12_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer) {
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
	if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_READ) || (parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_WRITE)) {
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	}
	else {
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	}

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = parameters->size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	kinc_microsoft_affirm(device->d3d12.device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                                    NULL, IID_GRAPHICS_PPV_ARGS(&buffer->d3d12.resource)));

	// buffer->impl.index_buffer_view.BufferLocation = buffer->impl.upload_buffer->GetGPUVirtualAddress();
	// buffer->impl.index_buffer_view.SizeInBytes = uploadBufferSize;
	// buffer->impl.index_buffer_view.Format = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	// buffer->impl.last_start = 0;
	// buffer->impl.last_count = kinc_g5_index_buffer_count(buffer);
}

void kope_d3d12_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	list->d3d12.device = &device->d3d12;
	kinc_microsoft_affirm(device->d3d12.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&list->d3d12.allocator)));
	kinc_microsoft_affirm(
	    device->d3d12.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->d3d12.allocator, NULL, IID_GRAPHICS_PPV_ARGS(&list->d3d12.list)));
}

static DXGI_FORMAT convert_texture_format(kope_g5_texture_format format) {
	switch (format) {
	case KOPE_G5_TEXTURE_FORMAT_R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;
	case KOPE_G5_TEXTURE_FORMAT_R8_SNORM:
		return DXGI_FORMAT_R8_SNORM;
	case KOPE_G5_TEXTURE_FORMAT_R8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case KOPE_G5_TEXTURE_FORMAT_R8_SINT:
		return DXGI_FORMAT_R8_SINT;
	case KOPE_G5_TEXTURE_FORMAT_R16_UINT:
		return DXGI_FORMAT_R16_UINT;
	case KOPE_G5_TEXTURE_FORMAT_R16_SINT:
		return DXGI_FORMAT_R16_SINT;
	case KOPE_G5_TEXTURE_FORMAT_R16_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_RG8_UNORM:
		return DXGI_FORMAT_R8G8_UNORM;
	case KOPE_G5_TEXTURE_FORMAT_RG8_SNORM:
		return DXGI_FORMAT_R8G8_SNORM;
	case KOPE_G5_TEXTURE_FORMAT_RG8_UINT:
		return DXGI_FORMAT_R8G8_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RG8_SINT:
		return DXGI_FORMAT_R8G8_SINT;
	case KOPE_G5_TEXTURE_FORMAT_R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case KOPE_G5_TEXTURE_FORMAT_R32_SINT:
		return DXGI_FORMAT_R32_SINT;
	case KOPE_G5_TEXTURE_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_RG16_UINT:
		return DXGI_FORMAT_R16G16_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RG16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case KOPE_G5_TEXTURE_FORMAT_RG16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_SNORM:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UINT:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_SINT:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case KOPE_G5_TEXTURE_FORMAT_RGB9E5U_FLOAT:
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UINT:
		return DXGI_FORMAT_R10G10B10A2_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UNORM:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case KOPE_G5_TEXTURE_FORMAT_RG11B10U_FLOAT:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_RG32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RG32_SINT:
		return DXGI_FORMAT_R32G32_SINT;
	case KOPE_G5_TEXTURE_FORMAT_RG32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_UINT:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_SINT:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	// case KOPE_G5_TEXTURE_FORMAT_STENCIL8:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM:
		return DXGI_FORMAT_D16_UNORM;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}

	assert(false);
	return DXGI_FORMAT_UNKNOWN;
}

static int format_byte_size(kope_g5_texture_format format) {
	switch (format) {
	case KOPE_G5_TEXTURE_FORMAT_R8_UNORM:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R8_SNORM:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R8_UINT:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R8_SINT:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R16_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R16_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R16_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG8_UNORM:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_RG8_SNORM:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_RG8_UINT:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_RG8_SINT:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_R32_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R32_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R32_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG16_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG16_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG16_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM_SRGB:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_SNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM_SRGB:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGB9E5U_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG11B10U_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG32_UINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RG32_SINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RG32_FLOAT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_UINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_SINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_FLOAT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_UINT:
		return 16;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_SINT:
		return 16;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_FLOAT:
		return 16;
	// case KOPE_G5_TEXTURE_FORMAT_STENCIL8:
	//	return 1;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24:
		return 8;
	}

	assert(false);
	return 4;
}

static D3D12_RESOURCE_DIMENSION convert_texture_dimension(kope_g5_texture_dimension dimension) {
	switch (dimension) {
	case KOPE_G5_TEXTURE_DIMENSION_1D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case KOPE_G5_TEXTURE_DIMENSION_2D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case KOPE_G5_TEXTURE_DIMENSION_3D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	}

	assert(false);
	return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
}

static D3D12_RTV_DIMENSION convert_texture_rtv_dimension(kope_g5_texture_dimension dimension) {
	switch (dimension) {
	case KOPE_G5_TEXTURE_DIMENSION_1D:
		return D3D12_RTV_DIMENSION_TEXTURE1D;
	case KOPE_G5_TEXTURE_DIMENSION_2D:
		return D3D12_RTV_DIMENSION_TEXTURE2D;
	case KOPE_G5_TEXTURE_DIMENSION_3D:
		return D3D12_RTV_DIMENSION_TEXTURE3D;
	}

	assert(false);
	return D3D12_RTV_DIMENSION_TEXTURE2D;

	// D3D12_RTV_DIMENSION_TEXTURE1D = 2, D3D12_RTV_DIMENSION_TEXTURE1DARRAY = 3, D3D12_RTV_DIMENSION_TEXTURE2D = 4, D3D12_RTV_DIMENSION_TEXTURE2DARRAY = 5,
	// D3D12_RTV_DIMENSION_TEXTURE2DMS = 6, D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY = 7, D3D12_RTV_DIMENSION_TEXTURE3D = 8
}

static D3D12_DSV_DIMENSION convert_texture_dsv_dimension(kope_g5_texture_dimension dimension) {
	switch (dimension) {
	case KOPE_G5_TEXTURE_DIMENSION_1D:
		return D3D12_DSV_DIMENSION_TEXTURE1D;
	case KOPE_G5_TEXTURE_DIMENSION_2D:
		return D3D12_DSV_DIMENSION_TEXTURE2D;
	case KOPE_G5_TEXTURE_DIMENSION_3D:
		assert(false);
		return D3D12_DSV_DIMENSION_TEXTURE2D;
	}

	assert(false);
	return D3D12_DSV_DIMENSION_TEXTURE2D;

	// D3D12_DSV_DIMENSION_UNKNOWN = 0, D3D12_DSV_DIMENSION_TEXTURE1D = 1, D3D12_DSV_DIMENSION_TEXTURE1DARRAY = 2, D3D12_DSV_DIMENSION_TEXTURE2D = 3,
	// D3D12_DSV_DIMENSION_TEXTURE2DARRAY = 4, D3D12_DSV_DIMENSION_TEXTURE2DMS = 5, D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY = 6
}

void kope_d3d12_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture) {
	DXGI_FORMAT format = convert_texture_format(parameters->format);
	int format_size = format_byte_size(parameters->format);

	D3D12_HEAP_PROPERTIES heap_properties;
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = convert_texture_dimension(parameters->dimension);
	desc.Alignment = 0;
	desc.Width = parameters->width;
	desc.Height = parameters->height;
	desc.DepthOrArraySize = parameters->depth_or_array_layers;
	desc.MipLevels = parameters->mip_level_count;
	desc.Format = format;
	desc.SampleDesc.Count = parameters->sample_count;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	kinc_microsoft_affirm(device->d3d12.device->CreateCommittedResource(
	    &heap_properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL, IID_GRAPHICS_PPV_ARGS(&texture->d3d12.resource)));

	texture->d3d12.rtv_index = 0xffffffff;
	texture->d3d12.dsv_index = 0xffffffff;

	if (parameters->usage & KONG_G5_TEXTURE_USAGE_RENDER_ATTACHMENT) {
		if (kope_g5_texture_format_is_depth(parameters->format)) {
			texture->d3d12.dsv_index = kope_index_allocator_allocate(&device->d3d12.dsv_index_allocator);

			D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
			desc.Format = format;
			desc.ViewDimension = convert_texture_dsv_dimension(parameters->dimension);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = device->d3d12.all_dsvs->GetCPUDescriptorHandleForHeapStart();
			dsv.ptr += texture->d3d12.rtv_index * device->d3d12.dsv_increment;

			device->d3d12.device->CreateDepthStencilView(texture->d3d12.resource, &desc, dsv);
		}
		else {
			texture->d3d12.rtv_index = kope_index_allocator_allocate(&device->d3d12.rtv_index_allocator);

			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			desc.Format = format;
			desc.ViewDimension = convert_texture_rtv_dimension(parameters->dimension);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv = device->d3d12.all_rtvs->GetCPUDescriptorHandleForHeapStart();
			rtv.ptr += texture->d3d12.rtv_index * device->d3d12.rtv_increment;

			device->d3d12.device->CreateRenderTargetView(texture->d3d12.resource, &desc, rtv);
		}
	}
}
