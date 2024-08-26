#ifndef KOPE_VULKAN_DEVICE_HEADER
#define KOPE_VULKAN_DEVICE_HEADER

#ifdef __cplusplus
extern "C" {
#endif

struct kope_g5_device;

typedef struct kope_vulkan_device {
	int nothing;
} kope_vulkan_device;

void kope_vulkan_device_create(struct kope_g5_device *device);

void kope_vulkan_device_destroy(struct kope_g5_device *device);

void kope_vulkan_device_set_name(struct kope_g5_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif
