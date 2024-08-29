#include "device.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/device_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/device_functions.h>
#endif

#include <assert.h>

void kope_g5_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
	KOPE_G5_CALL2(device_create, device, wishlist);
}

void kope_g5_device_destroy(kope_g5_device *device) {
	KOPE_G5_CALL1(device_destroy, device);
}

void kope_g5_device_set_name(kope_g5_device *device, const char *name) {
	KOPE_G5_CALL2(device_set_name, device, name);
}

void kope_g5_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer) {
	KOPE_G5_CALL3(device_create_buffer, device, parameters, buffer);
}

void kope_g5_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	KOPE_G5_CALL2(device_create_command_list, device, list);
}

void kope_g5_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture) {
#ifdef KOPE_G5_VALIDATION
	if (kope_g5_texture_format_is_depth(parameters->format)) {
		assert(parameters->dimension != KOPE_G5_TEXTURE_DIMENSION_3D);
	}
#endif
	KOPE_G5_CALL3(device_create_texture, device, parameters, texture);
}

void kope_g5_device_get_framebuffer_texture(kope_g5_device *device, uint32_t index, kope_g5_texture *texture) {
	KOPE_G5_CALL3(device_get_framebuffer_texture, device, index, texture);
}

void kope_g5_device_submit_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	KOPE_G5_CALL2(device_submit_command_list, device, list);
}

void kope_g5_device_swap_buffers(kope_g5_device *device) {
	KOPE_G5_CALL1(device_swap_buffers, device);
}

uint32_t kope_g5_texture_format_byte_size(kope_g5_texture_format format) {
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

bool kope_g5_texture_format_is_depth(kope_g5_texture_format format) {
	switch (format) {
	// case KOPE_G5_TEXTURE_FORMAT_STENCIL8:
	//	return 1;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24:
		return true;
	}
	return false;
}
