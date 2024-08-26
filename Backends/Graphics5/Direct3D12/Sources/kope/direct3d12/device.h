#ifndef KOPE_D3D12_DEVICE_HEADER
#define KOPE_D3D12_DEVICE_HEADER

#ifdef __cplusplus
extern "C" {
#endif

struct kope_g5_device;

typedef struct kope_d3d12_device {
	int nothing;
} kope_d3d12_device;

void kope_d3d12_device_create(struct kope_g5_device *device);

void kope_d3d12_device_destroy(struct kope_g5_device *device);

void kope_d3d12_device_set_name(struct kope_g5_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif
