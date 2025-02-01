#ifndef KOPE_G5_DEVICE_HEADER
#define KOPE_G5_DEVICE_HEADER

#include <kope/global.h>

#include <kinc/math/matrix.h>

#include "api.h"
#include "buffer.h"
#include "commandlist.h"
#include "fence.h"
#include "sampler.h"
#include "textureformat.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/device_structs.h>
#endif

#ifdef KOPE_METAL
#include <kope/metal/device_structs.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/device_structs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_device_wishlist {
	int nothing;
} kope_g5_device_wishlist;

typedef struct kope_g5_device {
	KOPE_G5_IMPL(device);
} kope_g5_device;

KOPE_FUNC void kope_g5_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist);

KOPE_FUNC void kope_g5_device_destroy(kope_g5_device *device);

KOPE_FUNC void kope_g5_device_set_name(kope_g5_device *device, const char *name);

typedef enum kope_g5_buffer_usage {
	KOPE_G5_BUFFER_USAGE_CPU_READ = 0x0001,
	KOPE_G5_BUFFER_USAGE_CPU_WRITE = 0x0002,
	KOPE_G5_BUFFER_USAGE_COPY_SRC = 0x0004,
	KOPE_G5_BUFFER_USAGE_COPY_DST = 0x0008,
	KOPE_G5_BUFFER_USAGE_INDEX = 0x0010,
	KOPE_G5_BUFFER_USAGE_READ_WRITE = 0x0020,
	KOPE_G5_BUFFER_USAGE_INDIRECT = 0x0040,
	KOPE_G5_BUFFER_USAGE_QUERY_RESOLVE = 0x0080,
	KOPE_G5_BUFFER_USAGE_RAYTRACING_VOLUME = 0x0100
} kope_g5_buffer_usage;

typedef struct kope_g5_buffer_parameters {
	uint64_t size;
	uint32_t usage_flags;
} kope_g5_buffer_parameters;

KOPE_FUNC void kope_g5_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer);

typedef enum kope_g5_texture_dimension { KOPE_G5_TEXTURE_DIMENSION_1D, KOPE_G5_TEXTURE_DIMENSION_2D, KOPE_G5_TEXTURE_DIMENSION_3D } kope_g5_texture_dimension;

typedef enum kope_g5_texture_usage {
	KONG_G5_TEXTURE_USAGE_COPY_SRC = 0x01,
	KONG_G5_TEXTURE_USAGE_COPY_DST = 0x02,
	KONG_G5_TEXTURE_USAGE_SAMPLE = 0x04,
	KONG_G5_TEXTURE_USAGE_READ_WRITE = 0x08,
	KONG_G5_TEXTURE_USAGE_RENDER_ATTACHMENT = 0x10,
	KONG_G5_TEXTURE_USAGE_FRAMEBUFFER = 0x20
} kope_g5_texture_usage;

typedef struct kope_g5_texture_parameters {
	uint32_t width;
	uint32_t height;
	uint32_t depth_or_array_layers;
	uint32_t mip_level_count;
	uint32_t sample_count;
	kope_g5_texture_dimension dimension;
	kope_g5_texture_format format;
	kope_g5_texture_usage usage;
	kope_g5_texture_format view_formats[8]; // necessary?
} kope_g5_texture_parameters;

KOPE_FUNC void kope_g5_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture);

#define KOPE_G5_MAX_FRAMEBUFFERS 3

KOPE_FUNC kope_g5_texture *kope_g5_device_get_framebuffer(kope_g5_device *device);

typedef enum kope_g5_address_mode { KOPE_G5_ADDRESS_MODE_CLAMP_TO_EDGE, KOPE_G5_ADDRESS_MODE_REPEAT, KOPE_G5_ADDRESS_MODE_MIRROR_REPEAT } kope_g5_address_mode;

typedef enum kope_g5_filter_mode { KOPE_G5_FILTER_MODE_NEAREST, KOPE_G5_FILTER_MODE_LINEAR } kope_g5_filter_mode;

typedef enum kope_g5_mipmap_filter_mode { KOPE_G5_MIPMAP_FILTER_MODE_NEAREST, KOPE_G5_MIPMAP_FILTER_MODE_LINEAR } kope_g5_mipmap_filter_mode;

typedef enum kope_g5_compare_function {
	KOPE_G5_COMPARE_FUNCTION_NEVER,
	KOPE_G5_COMPARE_FUNCTION_LESS,
	KOPE_G5_COMPARE_FUNCTION_EQUAL,
	KOPE_G5_COMPARE_FUNCTION_LESS_EQUAL,
	KOPE_G5_COMPARE_FUNCTION_GREATER,
	KOPE_G5_COMPARE_FUNCTION_NOT_EQUAL,
	KOPE_G5_COMPARE_FUNCTION_GREATER_EQUAL,
	KOPE_G5_COMPARE_FUNCTION_ALWAYS
} kope_g5_compare_function;

typedef struct kope_g5_sampler_parameters {
	kope_g5_address_mode address_mode_u;
	kope_g5_address_mode address_mode_v;
	kope_g5_address_mode address_mode_w;
	kope_g5_filter_mode mag_filter;
	kope_g5_filter_mode min_filter;
	kope_g5_mipmap_filter_mode mipmap_filter;
	float lod_min_clamp;
	float lod_max_clamp;
	kope_g5_compare_function compare;
	uint16_t max_anisotropy;
} kope_g5_sampler_parameters;

KOPE_FUNC void kope_g5_device_create_sampler(kope_g5_device *device, const kope_g5_sampler_parameters *parameters, kope_g5_sampler *sampler);

typedef enum kope_g5_command_list_type {
	KOPE_G5_COMMAND_LIST_TYPE_GRAPHICS,
	KOPE_G5_COMMAND_LIST_TYPE_COMPUTE,
	KOPE_G5_COMMAND_LIST_TYPE_COPY
} kope_g5_command_list_type;

KOPE_FUNC void kope_g5_device_create_command_list(kope_g5_device *device, kope_g5_command_list_type type, kope_g5_command_list *list);

typedef struct kope_g5_query_set {
	KOPE_G5_IMPL(query_set);
} kope_g5_query_set;

typedef enum kope_g5_query_type { KOPE_G5_QUERY_TYPE_OCCLUSION, KOPE_G5_QUERY_TYPE_TIMESTAMP } kope_g5_query_type;

typedef struct kope_g5_query_set_parameters {
	kope_g5_query_type type;
	uint32_t count;
} kope_g5_query_set_parameters;

KOPE_FUNC void kope_g5_device_create_query_set(kope_g5_device *device, const kope_g5_query_set_parameters *parameters, kope_g5_query_set *query_set);

KOPE_FUNC void kope_g5_device_create_fence(kope_g5_device *device, kope_g5_fence *fence);

KOPE_FUNC void kope_g5_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list);

KOPE_FUNC void kope_g5_device_wait_until_idle(kope_g5_device *device);

typedef struct kope_g5_raytracing_volume {
	KOPE_G5_IMPL(raytracing_volume);
} kope_g5_raytracing_volume;

typedef struct kope_g5_raytracing_hierarchy {
	KOPE_G5_IMPL(raytracing_hierarchy);
} kope_g5_raytracing_hierarchy;

KOPE_FUNC void kope_g5_device_create_raytracing_volume(kope_g5_device *device, kope_g5_buffer *vertex_buffer, uint64_t vertex_count,
                                                       kope_g5_buffer *index_buffer, uint32_t index_count, kope_g5_raytracing_volume *volume);

KOPE_FUNC void kope_g5_device_create_raytracing_hierarchy(kope_g5_device *device, kope_g5_raytracing_volume **volumes, kinc_matrix4x4_t *volume_transforms,
                                                          uint32_t volumes_count, kope_g5_raytracing_hierarchy *hierarchy);

KOPE_FUNC uint32_t kope_g5_device_align_texture_row_bytes(kope_g5_device *device, uint32_t row_bytes);

KOPE_FUNC void kope_g5_device_signal(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value);

KOPE_FUNC void kope_g5_device_wait(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif
