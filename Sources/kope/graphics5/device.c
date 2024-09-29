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
#ifdef KOPE_G5_VALIDATION
	buffer->usage_flags = parameters->usage_flags;
#endif
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

	texture->validation_format = parameters->format;
#endif
	KOPE_G5_CALL3(device_create_texture, device, parameters, texture);
}

kope_g5_texture *kope_g5_device_get_framebuffer(kope_g5_device *device) {
	return KOPE_G5_CALL1(device_get_framebuffer, device);
}

void kope_g5_device_create_query_set(kope_g5_device *device, const kope_g5_query_set_parameters *parameters, kope_g5_query_set *query_set) {
	KOPE_G5_CALL3(device_create_query_set, device, parameters, query_set);
}

void kope_g5_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	KOPE_G5_CALL2(device_execute_command_list, device, list);
}

void kope_g5_device_create_sampler(kope_g5_device *device, const kope_g5_sampler_parameters *parameters, kope_g5_sampler *sampler) {
	KOPE_G5_CALL3(device_create_sampler, device, parameters, sampler);
}

void kope_g5_device_create_raytracing_volume(kope_g5_device *device, kope_g5_buffer *vertex_buffer, uint64_t vertex_count, kope_g5_buffer *index_buffer,
                                             uint32_t index_count, kope_g5_raytracing_volume *volume) {
	KOPE_G5_CALL6(device_create_raytracing_volume, device, vertex_buffer, vertex_count, index_buffer, index_count, volume);
}

void kope_g5_device_create_raytracing_hierarchy(kope_g5_device *device, kope_g5_raytracing_volume **volumes, kinc_matrix4x4_t *volume_transforms,
                                                uint32_t volumes_count, kope_g5_raytracing_hierarchy *hierarchy) {
	KOPE_G5_CALL5(device_create_raytracing_hierarchy, device, volumes, volume_transforms, volumes_count, hierarchy);
}

void kope_g5_device_wait_until_idle(kope_g5_device *device) {
	KOPE_G5_CALL1(device_wait_until_idle, device);
}

uint32_t kope_g5_device_align_texture_row_bytes(kope_g5_device *device, uint32_t row_bytes) {
	return KOPE_G5_CALL2(device_align_texture_row_bytes, device, row_bytes);
}
