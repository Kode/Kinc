#ifndef KOPE_VULKAN_DEVICE_FUNCTIONS_HEADER
#define KOPE_VULKAN_DEVICE_FUNCTIONS_HEADER

#include <kope/graphics5/device.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_vulkan_device {
	int nothing;
} kope_vulkan_device;

void kope_vulkan_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist);

void kope_vulkan_device_destroy(kope_g5_device *device);

void kope_vulkan_device_set_name(kope_g5_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif
