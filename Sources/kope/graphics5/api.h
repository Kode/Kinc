#ifndef KOPE_G5_API_HEADER
#define KOPE_G5_API_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define KOPE_DIRECT3D12
// #define KOPE_VULKAN

typedef enum kope_g5_api { KOPE_G5_API_DIRECT3D12, KOPE_G5_API_VULKAN } kope_g5_api;

#if defined(KOPE_DIRECT3D12)

#if defined(KOPE_VULKAN)

#define KOPE_G5_IMPL(name)                                                                                                                                     \
	kope_d3d12_##name d3d12;                                                                                                                                   \
	kope_vulkan_##name vulkan
#define KOPE_G5_CALL(name)                                                                                                                                     \
	switch (selected_api) {                                                                                                                                    \
	case KOPE_G5_API_DIRECT3D12:                                                                                                                               \
		kope_d3d12_##name();                                                                                                                                   \
		break;                                                                                                                                                 \
	case KOPE_G5_API_VULKAN:                                                                                                                                   \
		kope_vulkan_##name();                                                                                                                                  \
		break;                                                                                                                                                 \
	}
#define KOPE_G5_CALL1(name, arg0)                                                                                                                              \
	switch (selected_api) {                                                                                                                                    \
	case KOPE_G5_API_DIRECT3D12:                                                                                                                               \
		kope_d3d12_##name(arg0);                                                                                                                               \
		break;                                                                                                                                                 \
	case KOPE_G5_API_VULKAN:                                                                                                                                   \
		kope_vulkan_##name(arg0);                                                                                                                              \
		break;                                                                                                                                                 \
	}
#define KOPE_G5_CALL2(name, arg0, arg1)                                                                                                                        \
	switch (selected_api) {                                                                                                                                    \
	case KOPE_G5_API_DIRECT3D12:                                                                                                                               \
		kope_d3d12_##name(arg0, arg1);                                                                                                                         \
		break;                                                                                                                                                 \
	case KOPE_G5_API_VULKAN:                                                                                                                                   \
		kope_vulkan_##name(arg0, arg1);                                                                                                                        \
		break;                                                                                                                                                 \
	}

#else

#define KOPE_G5_IMPL(name) kope_d3d12_##name d3d12
#define KOPE_G5_CALL(name) kope_d3d12_##name()
#define KOPE_G5_CALL1(name, arg0) kope_d3d12_##name(arg0)
#define KOPE_G5_CALL2(name, arg0, arg1) kope_d3d12_##name(arg0, arg1)

#endif

#elif defined(KOPE_VULKAN)

#define KOPE_G5_IMPL(name) kope_vulkan_##name d3d12
#define KOPE_G5_CALL(name) kope_vulkan_##name()
#define KOPE_G5_CALL1(name, arg0) kope_vulkan_##name(arg0)
#define KOPE_G5_CALL2(name, arg0, arg1) kope_vulkan_##name(arg0, arg1)

#endif

#ifdef __cplusplus
}
#endif

#endif
