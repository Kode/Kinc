#ifndef KOPE_G5_DEVICE_HEADER
#define KOPE_G5_DEVICE_HEADER

#include <kope/global.h>

#include "api.h"

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
	union {
		KOPE_G5_IMPL(device);
	};
} kope_g5_device;

KOPE_FUNC void kope_g5_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist);

KOPE_FUNC void kope_g5_device_destroy(kope_g5_device *device);

KOPE_FUNC void kope_g5_device_set_name(kope_g5_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif
