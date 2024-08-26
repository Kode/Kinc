#include "device.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/device_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/device_functions.h>
#endif

void kope_g5_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist) {
#ifdef KOPE_G5_VALIDATION
	do_stuff();
#endif
	KOPE_G5_CALL2(device_create, device, wishlist);
}

void kope_g5_device_destroy(kope_g5_device *device) {
#ifdef KOPE_G5_VALIDATION
	do_stuff();
#endif
	KOPE_G5_CALL1(device_destroy, device);
}

void kope_g5_device_set_name(kope_g5_device *device, const char *name) {
#ifdef KOPE_G5_VALIDATION
	do_stuff();
#endif
	KOPE_G5_CALL2(device_set_name, device, name);
}
