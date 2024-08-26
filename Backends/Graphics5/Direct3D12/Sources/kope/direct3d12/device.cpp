#include "device_functions.h"

#include <kope/graphics5/device.h>

#include <kinc/backend/SystemMicrosoft.h>

void kope_d3d12_device_create(kope_g5_device *device, kope_g5_device_wishlist wishlist) {
#ifdef _DEBUG
	ID3D12Debug *debug = NULL;
	if (D3D12GetDebugInterface(IID_PPV_ARGS(&debug)) == S_OK) {
		debug->EnableDebugLayer();
	}
#endif
	kinc_microsoft_affirm(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device->d3d12.device)));
}

void kope_d3d12_device_destroy(kope_g5_device *device) {
	device->d3d12.device->Release();
}

void kope_d3d12_device_set_name(kope_g5_device *device, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	device->d3d12.device->SetName(wstr);
}
