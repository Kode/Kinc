#ifndef KOPE_G5_DEVICE_HEADER
#define KOPE_G5_DEVICE_HEADER

#include <kope/global.h>

#include "api.h"
#include "buffer.h"
#include "commandlist.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/device_structs.h>
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
	KOPE_G5_BUFFER_USAGE_QUERY_RESOLVE = 0x0080
} kope_g5_buffer_usage;

typedef struct kope_g5_buffer_parameters {
	uint64_t size;
	uint32_t usage_flags;
} kope_g5_buffer_parameters;

KOPE_FUNC void kope_g5_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer);

typedef enum kope_g5_texture_dimension { KOPE_G5_TEXTURE_DIMENSION_1D, KOPE_G5_TEXTURE_DIMENSION_2D, KOPE_G5_TEXTURE_DIMENSION_3D } kope_g5_texture_dimension;

typedef enum kope_g5_texture_format {
	KOPE_G5_TEXTURE_FORMAT_R8_UNORM,
	KOPE_G5_TEXTURE_FORMAT_R8_SNORM,
	KOPE_G5_TEXTURE_FORMAT_R8_UINT,
	KOPE_G5_TEXTURE_FORMAT_R8_SINT,
	KOPE_G5_TEXTURE_FORMAT_R16_UINT,
	KOPE_G5_TEXTURE_FORMAT_R16_SINT,
	KOPE_G5_TEXTURE_FORMAT_R16_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RG8_UNORM,
	KOPE_G5_TEXTURE_FORMAT_RG8_SNORM,
	KOPE_G5_TEXTURE_FORMAT_RG8_UINT,
	KOPE_G5_TEXTURE_FORMAT_RG8_SINT,
	KOPE_G5_TEXTURE_FORMAT_R32_UINT,
	KOPE_G5_TEXTURE_FORMAT_R32_SINT,
	KOPE_G5_TEXTURE_FORMAT_R32_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RG16_UINT,
	KOPE_G5_TEXTURE_FORMAT_RG16_SINT,
	KOPE_G5_TEXTURE_FORMAT_RG16_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM,
	KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM_SRGB,
	KOPE_G5_TEXTURE_FORMAT_RGBA8_SNORM,
	KOPE_G5_TEXTURE_FORMAT_RGBA8_UINT,
	KOPE_G5_TEXTURE_FORMAT_RGBA8_SINT,
	KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM,
	KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM_SRGB,
	KOPE_G5_TEXTURE_FORMAT_RGB9E5U_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RGB10A2_UINT,
	KOPE_G5_TEXTURE_FORMAT_RGB10A2_UNORM,
	KOPE_G5_TEXTURE_FORMAT_RG11B10U_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RG32_UINT,
	KOPE_G5_TEXTURE_FORMAT_RG32_SINT,
	KOPE_G5_TEXTURE_FORMAT_RG32_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RGBA16_UINT,
	KOPE_G5_TEXTURE_FORMAT_RGBA16_SINT,
	KOPE_G5_TEXTURE_FORMAT_RGBA16_FLOAT,
	KOPE_G5_TEXTURE_FORMAT_RGBA32_UINT,
	KOPE_G5_TEXTURE_FORMAT_RGBA32_SINT,
	KOPE_G5_TEXTURE_FORMAT_RGBA32_FLOAT,
	// KOPE_G5_TEXTURE_FORMAT_STENCIL8, // not available in d3d12
	KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM,
	KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8,
	KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8,
	KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT,
	KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24,
	// TODO: compressed formats
} kope_g5_texture_format;

uint32_t kope_g5_texture_format_byte_size(kope_g5_texture_format format);

bool kope_g5_texture_format_is_depth(kope_g5_texture_format format);

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

KOPE_FUNC void kope_g5_device_get_framebuffer_texture(kope_g5_device *device, uint32_t index, kope_g5_texture *texture);

KOPE_FUNC void kope_g5_device_create_sampler(void *descriptor);

KOPE_FUNC void kope_g5_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list);

KOPE_FUNC void kope_g5_device_create_query_set(void *descriptor);

KOPE_FUNC void kope_g5_device_submit_command_list(kope_g5_device *device, kope_g5_command_list *list);

KOPE_FUNC void kope_g5_device_swap_buffers(kope_g5_device *device);

#ifdef __cplusplus
}
#endif

#endif
