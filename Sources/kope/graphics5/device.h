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

KOPE_FUNC void kope_g5_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist);

KOPE_FUNC void kope_g5_device_destroy(kope_g5_device *device);

KOPE_FUNC void kope_g5_device_set_name(kope_g5_device *device, const char *name);

typedef enum kope_g5_buffer_usage {
	KOPE_G5_BUFFER_USAGE_CPU_READ = 0x0001,
	KOPE_G5_BUFFER_USAGE_CPU_WRITE = 0x0002,
	KOPE_G5_BUFFER_USAGE_COPY_SRC = 0x0004,
	KOPE_G5_BUFFER_USAGE_COPY_DST = 0x0008,
	KOPE_G5_BUFFER_USAGE_INDEX = 0x0010,
	KOPE_G5_BUFFER_USAGE_WRITE = 0x0020,
	KOPE_G5_BUFFER_USAGE_INDIRECT = 0x0040,
	KOPE_G5_BUFFER_USAGE_QUERY_RESOLVE = 0x0080
} kope_g5_buffer_usage;

typedef struct kope_g5_buffer_parameters {
	uint64_t size;
	uint32_t usage_flags;
} kope_g5_buffer_parameters;

KOPE_FUNC void kope_g5_device_create_buffer(kope_g5_device *device, kope_g5_buffer_parameters parameters, kope_g5_buffer *buffer);

KOPE_FUNC void kope_g5_device_create_texture(void *descriptor);

KOPE_FUNC void kope_g5_device_create_sampler(void *descriptor);

KOPE_FUNC void kope_g5_device_create_command_list(kope_g5_device *device, kope_g5_command_list *list);

KOPE_FUNC void kope_g5_device_create_query_set(void *descriptor);

#ifdef __cplusplus
}
#endif

#endif
