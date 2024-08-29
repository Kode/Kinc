#include "device.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/device_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/device_functions.h>
#endif

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
#ifdef KOPE_VALIDATION
	if (kope_g5_texture_format_is_depth(parameters->format)) {
		assert(parameters->dimension != KOPE_G5_TEXTURE_DIMENSION_3D);
	}
#endif
	KOPE_G5_CALL3(device_create_texture, device, parameters, texture);
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
