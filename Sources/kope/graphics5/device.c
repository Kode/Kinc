#include "device.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/device_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/device_functions.h>
#endif

void kope_g5_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist) {
	KOPE_G5_CALL2(device_create, device, wishlist);
}

void kope_g5_device_destroy(kope_g5_device *device) {
	KOPE_G5_CALL1(device_destroy, device);
}

void kope_g5_device_set_name(kope_g5_device *device, const char *name) {
	KOPE_G5_CALL2(device_set_name, device, name);
}

void kope_g5_device_create_buffer(kope_g5_device *device, kope_g5_buffer_parameters parameters, kope_g5_buffer *buffer) {
	KOPE_G5_CALL3(device_create_buffer, device, parameters, buffer);
}

void kope_g5_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	KOPE_G5_CALL2(device_create_command_list, device, list);
}

void kope_g5_device_create_texture(kope_g5_device *device, kope_g5_texture_parameters parameters, kope_g5_texture *texture) {
	KOPE_G5_CALL3(device_create_texture, device, parameters, texture);
}
