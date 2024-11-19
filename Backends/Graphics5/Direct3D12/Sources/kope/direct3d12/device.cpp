#include "device_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/device.h>
#include <kope/util/align.h>

#include <kinc/backend/SystemMicrosoft.h>
#include <kinc/backend/Windows.h>

#include <kinc/log.h>
#include <kinc/window.h>

#include <assert.h>

#include <dxgi1_4.h>

#if defined(KOPE_NVAPI) && !defined(NDEBUG)

#include <nvapi.h>

static void __stdcall myValidationMessageCallback(void *pUserData, NVAPI_D3D12_RAYTRACING_VALIDATION_MESSAGE_SEVERITY severity, const char *messageCode,
                                                  const char *message, const char *messageDetails) {
	const char *severityString = "unknown";
	switch (severity) {
	case NVAPI_D3D12_RAYTRACING_VALIDATION_MESSAGE_SEVERITY_ERROR:
		severityString = "error";
		break;
	case NVAPI_D3D12_RAYTRACING_VALIDATION_MESSAGE_SEVERITY_WARNING:
		severityString = "warning";
		break;
	}
	kinc_log(KINC_LOG_LEVEL_ERROR, "Ray Tracing Validation message: %s: [%s] %s\n%s", severityString, messageCode, message, messageDetails);
}
#endif

void kope_d3d12_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
#ifndef NDEBUG
	ID3D12Debug *debug = NULL;
	if (D3D12GetDebugInterface(IID_PPV_ARGS(&debug)) == S_OK) {
		debug->EnableDebugLayer();
	}
#endif

	IDXGIFactory4 *dxgi_factory = NULL;
	kinc_microsoft_affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));

	HRESULT result = S_FALSE;

#ifndef KOPE_D3D12_FORCE_WARP
	result = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device->d3d12.device));
	if (result == S_OK) {
		kinc_log(KINC_LOG_LEVEL_INFO, "%s", "Direct3D running on feature level 12.2.");
	}

	if (result != S_OK) {
		result = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device->d3d12.device));
		if (result == S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", "Direct3D running on feature level 12.1.");
		}
	}

	if (result != S_OK) {
		result = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device->d3d12.device));
		if (result == S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", "Direct3D running on feature level 12.0.");
		}
	}
#endif

	if (result != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "%s", "Falling back to the WARP driver, things will be slow.");

		IDXGIAdapter *adapter;
		dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		
		result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device->d3d12.device));
		if (result == S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", "Direct3D running on feature level 12.2.");
		}

		if (result != S_OK) {
			result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device->d3d12.device));
			if (result == S_OK) {
				kinc_log(KINC_LOG_LEVEL_INFO, "%s", "Direct3D running on feature level 12.1.");
			}
		}

		if (result != S_OK) {
			result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device->d3d12.device));
			if (result == S_OK) {
				kinc_log(KINC_LOG_LEVEL_INFO, "%s", "Direct3D running on feature level 12.0.");
			}
		}
	}

	assert(result == S_OK);

#if defined(KOPE_NVAPI) && !defined(NDEBUG)
	NvAPI_Initialize();
	NvAPI_D3D12_EnableRaytracingValidation(device->d3d12.device, NVAPI_D3D12_RAYTRACING_VALIDATION_FLAG_NONE);
	void *handle = nullptr;
	NvAPI_D3D12_RegisterRaytracingValidationMessageCallback(device->d3d12.device, &myValidationMessageCallback, nullptr, &handle);
#endif

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

		kinc_microsoft_affirm(dxgi_factory->CreateSwapChain((IUnknown *)device->d3d12.queue, &desc, &device->d3d12.swap_chain));
	}

	device->d3d12.cbv_srv_uav_increment = device->d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->d3d12.sampler_increment = device->d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = KOPE_INDEX_ALLOCATOR_SIZE;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&device->d3d12.all_samplers)));

		kope_index_allocator_init(&device->d3d12.sampler_index_allocator);
	}

	for (int i = 0; i < KOPE_D3D12_FRAME_COUNT; ++i) {
		device->d3d12.swap_chain->GetBuffer(i, IID_GRAPHICS_PPV_ARGS(&device->d3d12.framebuffer_textures[i].d3d12.resource));

		device->d3d12.framebuffer_textures[i].d3d12.width = kinc_window_width(0);
		device->d3d12.framebuffer_textures[i].d3d12.height = kinc_window_height(0);
		device->d3d12.framebuffer_textures[i].d3d12.depth_or_array_layers = 1;
		device->d3d12.framebuffer_textures[i].d3d12.mip_level_count = 1;

		device->d3d12.framebuffer_textures[i].d3d12.in_flight_frame_index = 0;

#ifdef KOPE_G5_VALIDATION
		device->d3d12.framebuffer_textures[i].validation_format = KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM;
#endif

		kope_g5_texture_parameters parameters = {};
		parameters.format = KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM;
		parameters.dimension = KOPE_G5_TEXTURE_DIMENSION_2D;
		parameters.usage = KONG_G5_TEXTURE_USAGE_RENDER_ATTACHMENT;

		device->d3d12.framebuffer_textures[i].d3d12.resource_states[0] = D3D12_RESOURCE_STATE_PRESENT;
	}

	device->d3d12.framebuffer_index = 0;

	device->d3d12.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&device->d3d12.frame_fence));
	device->d3d12.frame_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	device->d3d12.current_frame_index = 1;

	device->d3d12.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&device->d3d12.execution_fence));
	device->d3d12.execution_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	device->d3d12.execution_index = 1;

	{
		const uint32_t descriptor_count = 1024 * 10;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = descriptor_count;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&device->d3d12.descriptor_heap)));

		oa_create(&device->d3d12.descriptor_heap_allocator, descriptor_count, 4096);
	}

	{
		const uint32_t sampler_count = 1024;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = sampler_count;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&device->d3d12.sampler_heap)));

		oa_create(&device->d3d12.sampler_heap_allocator, sampler_count, 4096);
	}

	/*{
	    kope_d3d12_device_create_command_list(device, &device->d3d12.management_list);

	    kope_d3d12_execution_context *execution_context = &device->d3d12.execution_contexts[device->d3d12.execution_context_index];
	    ID3D12DescriptorHeap *heaps[] = {execution_context->descriptor_heap, execution_context->sampler_heap};
	    device->d3d12.management_list.d3d12.list->SetDescriptorHeaps(2, heaps);

	    kope_g5_device_execute_command_list(device, &device->d3d12.management_list);
	}*/
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

	D3D12_HEAP_PROPERTIES props;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	props.CreationNodeMask = 1;
	props.VisibleNodeMask = 1;
	if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_WRITE) != 0) {
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
	}
	else if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_READ) != 0) {
		props.Type = D3D12_HEAP_TYPE_READBACK;
	}
	else {
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
	}

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_RAYTRACING_VOLUME) != 0 || (parameters->usage_flags & KOPE_G5_BUFFER_USAGE_READ_WRITE) != 0) {
		desc.Width = parameters->size;
	}
	else {
		desc.Width = align_pow2((int)parameters->size, 256); // 256 required for CBVs
	}
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	buffer->d3d12.size = parameters->size;

	if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_READ_WRITE) != 0) {
		buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_COMMON;
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	else if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_READ) != 0) {
		buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	else {
		buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	if ((parameters->usage_flags & KOPE_G5_BUFFER_USAGE_RAYTRACING_VOLUME) != 0) {
		buffer->d3d12.resource_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	buffer->d3d12.device = device;

	buffer->d3d12.ranges_count = 0;

	buffer->d3d12.cpu_read = (parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_READ) != 0;
	buffer->d3d12.cpu_write = (parameters->usage_flags & KOPE_G5_BUFFER_USAGE_CPU_WRITE) != 0;

	kinc_microsoft_affirm(device->d3d12.device->CreateCommittedResource(
	    &props, D3D12_HEAP_FLAG_NONE, &desc, (D3D12_RESOURCE_STATES)buffer->d3d12.resource_state, NULL, IID_GRAPHICS_PPV_ARGS(&buffer->d3d12.resource)));
}

static uint8_t command_list_oldest_allocator(kope_g5_command_list *list) {
	uint64_t lowest_execution_index = UINT64_MAX;
	uint8_t allocator_index = 255;

	for (uint8_t i = 0; i < KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT; ++i) {
		if (list->d3d12.allocator_execution_index[i] < lowest_execution_index) {
			allocator_index = i;
			lowest_execution_index = list->d3d12.allocator_execution_index[i];
		}
	}

	return allocator_index;
}

void kope_d3d12_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	list->d3d12.device = &device->d3d12;

	for (int i = 0; i < KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT; ++i) {
		kinc_microsoft_affirm(device->d3d12.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&list->d3d12.allocator[i])));
		list->d3d12.allocator_execution_index[i] = 0;

		oa_allocate(&device->d3d12.descriptor_heap_allocator, KOPE_D3D12_COMMAND_LIST_DYNAMIC_DESCRIPTORS_COUNT,
		            &list->d3d12.dynamic_descriptor_allocations[i]);
		list->d3d12.dynamic_descriptor_offsets[i] = 0;
	}

	list->d3d12.current_allocator_index = 0;

	kinc_microsoft_affirm(device->d3d12.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->d3d12.allocator[list->d3d12.current_allocator_index],
	                                                              NULL, IID_GRAPHICS_PPV_ARGS(&list->d3d12.list)));

	list->d3d12.compute_pipe = NULL;
	list->d3d12.ray_pipe = NULL;
	list->d3d12.render_pipe = NULL;

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&list->d3d12.rtv_descriptors)));

		list->d3d12.rtv_increment = device->d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->d3d12.device->CreateDescriptorHeap(&desc, IID_GRAPHICS_PPV_ARGS(&list->d3d12.dsv_descriptor)));

		list->d3d12.dsv_increment = device->d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	list->d3d12.occlusion_query_set = NULL;
	list->d3d12.current_occlusion_query_index = 0;

	list->d3d12.timestamp_query_set = NULL;
	list->d3d12.timestamp_beginning_of_pass_write_index = 0;
	list->d3d12.timestamp_end_of_pass_write_index = 0;

	list->d3d12.blocking_frame_index = 0;

	list->d3d12.queued_buffer_accesses_count = 0;

	list->d3d12.queued_descriptor_set_accesses_count = 0;

	list->d3d12.presenting = false;

	ID3D12DescriptorHeap *heaps[] = {list->d3d12.device->descriptor_heap, list->d3d12.device->sampler_heap};
	list->d3d12.list->SetDescriptorHeaps(2, heaps);
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

void kope_d3d12_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture) {
	DXGI_FORMAT format = convert_texture_format(parameters->format);

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

	D3D12_CLEAR_VALUE optimizedClearValue;
	D3D12_CLEAR_VALUE *optimizedClearValuePointer = NULL;

	if ((parameters->usage & KONG_G5_TEXTURE_USAGE_RENDER_ATTACHMENT) != 0) {
		if (kope_g5_texture_format_is_depth(parameters->format)) {
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			optimizedClearValue.DepthStencil.Depth = 1.0f;
			optimizedClearValue.DepthStencil.Stencil = 0;
		}
		else {
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			optimizedClearValue.Color[0] = 0.0f;
			optimizedClearValue.Color[1] = 0.0f;
			optimizedClearValue.Color[2] = 0.0f;
			optimizedClearValue.Color[3] = 1.0f;
		}

		optimizedClearValue.Format = format;

		optimizedClearValuePointer = &optimizedClearValue;
	}

	if ((parameters->usage & KONG_G5_TEXTURE_USAGE_READ_WRITE) != 0) {
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	kinc_microsoft_affirm(device->d3d12.device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc,
	                                                                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, optimizedClearValuePointer,
	                                                                    IID_GRAPHICS_PPV_ARGS(&texture->d3d12.resource)));

	texture->d3d12.format = format;

	texture->d3d12.width = parameters->width;
	texture->d3d12.height = parameters->height;
	texture->d3d12.depth_or_array_layers = parameters->depth_or_array_layers;
	texture->d3d12.mip_level_count = parameters->mip_level_count;

	texture->d3d12.in_flight_frame_index = 0;

	for (uint32_t array_layer = 0; array_layer < parameters->depth_or_array_layers; ++array_layer) {
		for (uint32_t mip_level = 0; mip_level < parameters->mip_level_count; ++mip_level) {
			texture->d3d12.resource_states[kope_d3d12_texture_resource_state_index(texture, mip_level, array_layer)] =
			    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}
	}
}

kope_g5_texture *kope_d3d12_device_get_framebuffer(kope_g5_device *device) {
	return &device->d3d12.framebuffer_textures[device->d3d12.framebuffer_index];
}

static void wait_for_fence(kope_g5_device *device, ID3D12Fence *fence, HANDLE event, UINT64 completion_value) {
	if (fence->GetCompletedValue() < completion_value) {
		kinc_microsoft_affirm(fence->SetEventOnCompletion(completion_value, event));
		WaitForSingleObject(event, INFINITE);

#if defined(KOPE_NVAPI) && !defined(NDEBUG)
		NvAPI_D3D12_FlushRaytracingValidationMessages(device->d3d12.device);
#endif
	}
}

static bool check_for_fence(ID3D12Fence *fence, UINT64 completion_value) {
	// kinc_log(KINC_LOG_LEVEL_INFO, "Done: %llu Check: %llu", fence->GetCompletedValue(), completion_value);
	return fence->GetCompletedValue() >= completion_value;
}

static void wait_for_frame(kope_g5_device *device, uint64_t frame_index) {
	wait_for_fence(device, device->d3d12.frame_fence, device->d3d12.frame_event, frame_index);
}

static void clean_buffer_accesses(kope_g5_buffer *buffer, uint64_t finished_execution_index) {
	kope_d3d12_buffer_range ranges[KOPE_D3D12_MAX_BUFFER_RANGES];
	uint32_t ranges_count = 0;

	for (uint32_t range_index = 0; range_index < buffer->d3d12.ranges_count; ++range_index) {
		if (buffer->d3d12.ranges[range_index].execution_index > finished_execution_index) {
			ranges[ranges_count] = buffer->d3d12.ranges[range_index];
			ranges_count += 1;
		}
	}

	memcpy(&buffer->d3d12.ranges, &ranges, sizeof(ranges));
	buffer->d3d12.ranges_count = ranges_count;
}

void kope_d3d12_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	if (list->d3d12.presenting) {
		kope_g5_texture *framebuffer = kope_d3d12_device_get_framebuffer(device);
		if (framebuffer->d3d12.resource_states[0] != D3D12_RESOURCE_STATE_PRESENT) {
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource = framebuffer->d3d12.resource;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)framebuffer->d3d12.resource_states[0];
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.Subresource = 0;

			list->d3d12.list->ResourceBarrier(1, &barrier);

			framebuffer->d3d12.resource_states[0] = D3D12_RESOURCE_STATE_PRESENT;
		}
	}

	if (list->d3d12.blocking_frame_index > 0) {
		wait_for_frame(device, list->d3d12.blocking_frame_index);
		list->d3d12.blocking_frame_index = 0;
	}

	list->d3d12.list->Close();

	for (uint32_t buffer_access_index = 0; buffer_access_index < list->d3d12.queued_buffer_accesses_count; ++buffer_access_index) {
		kope_d3d12_buffer_access access = list->d3d12.queued_buffer_accesses[buffer_access_index];
		kope_g5_buffer *buffer = access.buffer;

		clean_buffer_accesses(buffer, device->d3d12.execution_fence->GetCompletedValue());

		assert(buffer->d3d12.ranges_count < KOPE_D3D12_MAX_BUFFER_RANGES);
		buffer->d3d12.ranges[buffer->d3d12.ranges_count].execution_index = device->d3d12.execution_index;
		buffer->d3d12.ranges[buffer->d3d12.ranges_count].offset = access.offset;
		buffer->d3d12.ranges[buffer->d3d12.ranges_count].size = access.size;
		buffer->d3d12.ranges_count += 1;
	}
	list->d3d12.queued_buffer_accesses_count = 0;

	for (uint32_t set_access_index = 0; set_access_index < list->d3d12.queued_descriptor_set_accesses_count; ++set_access_index) {
		kope_d3d12_descriptor_set *set = list->d3d12.queued_descriptor_set_accesses[set_access_index];

		set->execution_index = device->d3d12.execution_index;
	}
	list->d3d12.queued_descriptor_set_accesses_count = 0;

	list->d3d12.allocator_execution_index[list->d3d12.current_allocator_index] = device->d3d12.execution_index;

	ID3D12CommandList *lists[] = {list->d3d12.list};
	device->d3d12.queue->ExecuteCommandLists(1, lists);

	device->d3d12.queue->Signal(device->d3d12.execution_fence, device->d3d12.execution_index);

	uint8_t allocator_index = command_list_oldest_allocator(list);
	list->d3d12.current_allocator_index = allocator_index;
	uint64_t allocator_execution_index = list->d3d12.allocator_execution_index[allocator_index];

	wait_for_fence(device, device->d3d12.execution_fence, device->d3d12.execution_event, allocator_execution_index);

	list->d3d12.dynamic_descriptor_offsets[allocator_index] = 0;
	list->d3d12.allocator[allocator_index]->Reset();
	list->d3d12.list->Reset(list->d3d12.allocator[allocator_index], NULL);

	ID3D12DescriptorHeap *heaps[] = {list->d3d12.device->descriptor_heap, list->d3d12.device->sampler_heap};
	list->d3d12.list->SetDescriptorHeaps(2, heaps);

	if (list->d3d12.presenting) {
		kope_g5_texture *framebuffer = kope_d3d12_device_get_framebuffer(device);
		framebuffer->d3d12.in_flight_frame_index = device->d3d12.current_frame_index;

		kinc_microsoft_affirm(device->d3d12.swap_chain->Present(1, 0));

		device->d3d12.queue->Signal(device->d3d12.frame_fence, device->d3d12.current_frame_index);

		device->d3d12.current_frame_index += 1;
		device->d3d12.framebuffer_index = (device->d3d12.framebuffer_index + 1) % KOPE_D3D12_FRAME_COUNT;

		list->d3d12.presenting = false;
	}

	device->d3d12.execution_index += 1;
}

void kope_d3d12_device_wait_until_idle(kope_g5_device *device) {
	wait_for_fence(device, device->d3d12.execution_fence, device->d3d12.execution_event, device->d3d12.execution_index - 1);
}

void kope_d3d12_device_create_descriptor_set(kope_g5_device *device, uint32_t descriptor_count, uint32_t dynamic_descriptor_count,
                                             uint32_t bindless_descriptor_count, uint32_t sampler_count, kope_d3d12_descriptor_set *set) {
	if (descriptor_count > 0) {
		oa_allocate(&device->d3d12.descriptor_heap_allocator, descriptor_count, &set->descriptor_allocation);
	}
	set->descriptor_count = descriptor_count;

	set->dynamic_descriptor_count = dynamic_descriptor_count;

	if (bindless_descriptor_count > 0) {
		oa_allocate(&device->d3d12.descriptor_heap_allocator, bindless_descriptor_count, &set->bindless_descriptor_allocation);
	}
	set->bindless_descriptor_count = bindless_descriptor_count;

	if (sampler_count > 0) {
		oa_allocate(&device->d3d12.sampler_heap_allocator, sampler_count, &set->sampler_allocation);
	}
	set->sampler_count = sampler_count;

	set->execution_index = 0;
}

static D3D12_TEXTURE_ADDRESS_MODE convert_address_mode(kope_g5_address_mode mode) {
	switch (mode) {
	case KOPE_G5_ADDRESS_MODE_CLAMP_TO_EDGE:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case KOPE_G5_ADDRESS_MODE_REPEAT:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case KOPE_G5_ADDRESS_MODE_MIRROR_REPEAT:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	default:
		assert(false);
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}

void kope_d3d12_device_create_sampler(kope_g5_device *device, const kope_g5_sampler_parameters *parameters, kope_g5_sampler *sampler) {
	sampler->d3d12.sampler_index = kope_index_allocator_allocate(&device->d3d12.sampler_index_allocator);

	D3D12_SAMPLER_DESC desc = {};
	desc.Filter =
	    parameters->max_anisotropy > 1 ? D3D12_FILTER_ANISOTROPIC : convert_filter(parameters->min_filter, parameters->mag_filter, parameters->mipmap_filter);
	desc.AddressU = convert_address_mode(parameters->address_mode_u);
	desc.AddressV = convert_address_mode(parameters->address_mode_v);
	desc.AddressW = convert_address_mode(parameters->address_mode_w);
	desc.MinLOD = parameters->lod_min_clamp;
	desc.MaxLOD = parameters->lod_max_clamp;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = parameters->max_anisotropy;
	desc.ComparisonFunc = convert_compare_function(parameters->compare);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = device->d3d12.all_samplers->GetCPUDescriptorHandleForHeapStart();
	descriptor_handle.ptr += sampler->d3d12.sampler_index * device->d3d12.sampler_increment;
	device->d3d12.device->CreateSampler(&desc, descriptor_handle);
}

void kope_d3d12_device_create_raytracing_volume(kope_g5_device *device, kope_g5_buffer *vertex_buffer, uint64_t vertex_count, kope_g5_buffer *index_buffer,
                                                uint32_t index_count, kope_g5_raytracing_volume *volume) {
	volume->d3d12.vertex_buffer = vertex_buffer;
	volume->d3d12.vertex_count = vertex_count;
	volume->d3d12.index_buffer = index_buffer;
	volume->d3d12.index_count = index_count;

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

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info = {};
	device->d3d12.device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild_info);

	kope_g5_buffer_parameters scratch_params;
	scratch_params.size = prebuild_info.ScratchDataSizeInBytes;
	scratch_params.usage_flags = KOPE_G5_BUFFER_USAGE_READ_WRITE;
	kope_g5_device_create_buffer(device, &scratch_params, &volume->d3d12.scratch_buffer); // TODO: delete later

	kope_g5_buffer_parameters as_params;
	as_params.size = prebuild_info.ResultDataMaxSizeInBytes;
	as_params.usage_flags = KOPE_G5_BUFFER_USAGE_READ_WRITE | KOPE_G5_BUFFER_USAGE_RAYTRACING_VOLUME;
	kope_g5_device_create_buffer(device, &as_params, &volume->d3d12.acceleration_structure);
}

void kope_d3d12_device_create_raytracing_hierarchy(kope_g5_device *device, kope_g5_raytracing_volume **volumes, kinc_matrix4x4_t *volume_transforms,
                                                   uint32_t volumes_count, kope_g5_raytracing_hierarchy *hierarchy) {
	hierarchy->d3d12.volumes_count = volumes_count;

	kope_g5_buffer_parameters instances_params;
	instances_params.size = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * hierarchy->d3d12.volumes_count;
	instances_params.usage_flags = KOPE_G5_BUFFER_USAGE_CPU_WRITE;
	kope_g5_device_create_buffer(device, &instances_params, &hierarchy->d3d12.instances);

	D3D12_RAYTRACING_INSTANCE_DESC *descs = (D3D12_RAYTRACING_INSTANCE_DESC *)kope_g5_buffer_lock_all(&hierarchy->d3d12.instances);
	for (uint32_t volume_index = 0; volume_index < hierarchy->d3d12.volumes_count; ++volume_index) {
		memset(&descs[volume_index], 0, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		descs[volume_index].InstanceID = volume_index;
		descs[volume_index].InstanceMask = 1;
		descs[volume_index].AccelerationStructure = volumes[volume_index]->d3d12.acceleration_structure.d3d12.resource->GetGPUVirtualAddress();
	}

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
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = hierarchy->d3d12.volumes_count;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = hierarchy->d3d12.instances.d3d12.resource->GetGPUVirtualAddress();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info = {};
	device->d3d12.device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild_info);

	kope_g5_buffer_parameters scratch_params;
	scratch_params.size = prebuild_info.ScratchDataSizeInBytes;
	scratch_params.usage_flags = KOPE_G5_BUFFER_USAGE_READ_WRITE;
	kope_g5_device_create_buffer(device, &scratch_params, &hierarchy->d3d12.scratch_buffer); // TODO: delete later

	kope_g5_buffer_parameters update_scratch_params;
	update_scratch_params.size = prebuild_info.UpdateScratchDataSizeInBytes > 0 ? prebuild_info.UpdateScratchDataSizeInBytes : 1;
	update_scratch_params.usage_flags = KOPE_G5_BUFFER_USAGE_READ_WRITE;
	kope_g5_device_create_buffer(device, &update_scratch_params, &hierarchy->d3d12.update_scratch_buffer);

	kope_g5_buffer_parameters as_params;
	as_params.size = prebuild_info.ResultDataMaxSizeInBytes;
	as_params.usage_flags = KOPE_G5_BUFFER_USAGE_READ_WRITE | KOPE_G5_BUFFER_USAGE_RAYTRACING_VOLUME;
	kope_g5_device_create_buffer(device, &as_params, &hierarchy->d3d12.acceleration_structure);
}

void kope_d3d12_device_create_query_set(kope_g5_device *device, const kope_g5_query_set_parameters *parameters, kope_g5_query_set *query_set) {
	D3D12_QUERY_HEAP_DESC desc = {};
	desc.Type = parameters->type == KOPE_G5_QUERY_TYPE_OCCLUSION ? D3D12_QUERY_HEAP_TYPE_OCCLUSION : D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	desc.Count = parameters->count;
	desc.NodeMask = 0;

	query_set->d3d12.query_type = (uint8_t)parameters->type;
	device->d3d12.device->CreateQueryHeap(&desc, IID_GRAPHICS_PPV_ARGS(&query_set->d3d12.query_heap));
}

uint32_t kope_d3d12_device_align_texture_row_bytes(kope_g5_device *device, uint32_t row_bytes) {
	return (uint32_t)align_pow2((int)row_bytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
}
