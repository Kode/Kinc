#ifndef KOPE_G5_API_HEADER
#define KOPE_G5_API_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define KOPE_DIRECT3D12
// #define KOPE_VULKAN

#ifndef NDEBUG
#define KOPE_G5_VALIDATION
#endif

typedef enum kope_g5_api { KOPE_G5_API_DIRECT3D12, KOPE_G5_API_VULKAN } kope_g5_api;

#if defined(KOPE_DIRECT3D12)

#if defined(KOPE_VULKAN)

#define KOPE_G5_IMPL(name)                                                                                                                                     \
	union {                                                                                                                                                    \
		kope_d3d12_##name d3d12;                                                                                                                               \
		kope_vulkan_##name vulkan;                                                                                                                             \
	}
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
#define KOPE_G5_CALL3(name, arg0, arg1, arg2)                                                                                                                  \
	switch (selected_api) {                                                                                                                                    \
	case KOPE_G5_API_DIRECT3D12:                                                                                                                               \
		kope_d3d12_##name(arg0, arg1, arg2);                                                                                                                   \
		break;                                                                                                                                                 \
	case KOPE_G5_API_VULKAN:                                                                                                                                   \
		kope_vulkan_##name(arg0, arg1, arg2);                                                                                                                  \
		break;                                                                                                                                                 \
	}                                                                                                                                                          \
	#define KOPE_G5_CALL4(name, arg0, arg1, arg2, arg3) switch (selected_api) {                                                                                \
	case KOPE_G5_API_DIRECT3D12:                                                                                                                               \
		kope_d3d12_##name(arg0, arg1, arg2, arg3);                                                                                                             \
		break;                                                                                                                                                 \
	case KOPE_G5_API_VULKAN:                                                                                                                                   \
		kope_vulkan_##name(arg0, arg1, arg2, arg3);                                                                                                            \
		break;                                                                                                                                                 \
	}

#else

#define KOPE_G5_IMPL(name) kope_d3d12_##name d3d12
#define KOPE_G5_CALL(name) kope_d3d12_##name()
#define KOPE_G5_CALL1(name, arg0) kope_d3d12_##name(arg0)
#define KOPE_G5_CALL2(name, arg0, arg1) kope_d3d12_##name(arg0, arg1)
#define KOPE_G5_CALL3(name, arg0, arg1, arg2) kope_d3d12_##name(arg0, arg1, arg2)
#define KOPE_G5_CALL4(name, arg0, arg1, arg2, arg3) kope_d3d12_##name(arg0, arg1, arg2, arg3)
#define KOPE_G5_CALL5(name, arg0, arg1, arg2, arg3, arg4) kope_d3d12_##name(arg0, arg1, arg2, arg3, arg4)
#define KOPE_G5_CALL6(name, arg0, arg1, arg2, arg3, arg4, arg5) kope_d3d12_##name(arg0, arg1, arg2, arg3, arg4, arg5)
#define KOPE_G5_CALL7(name, arg0, arg1, arg2, arg3, arg4, arg5, arg6) kope_d3d12_##name(arg0, arg1, arg2, arg3, arg4, arg5, arg6)

#endif

#elif defined(KOPE_VULKAN)

#define KOPE_G5_IMPL(name) kope_vulkan_##name d3d12
#define KOPE_G5_CALL(name) kope_vulkan_##name()
#define KOPE_G5_CALL1(name, arg0) kope_vulkan_##name(arg0)
#define KOPE_G5_CALL2(name, arg0, arg1) kope_vulkan_##name(arg0, arg1)
#define KOPE_G5_CALL3(name, arg0, arg1, arg2) kope_vulkan_##name(arg0, arg1, arg2)
#define KOPE_G5_CALL4(name, arg0, arg1, arg2, arg3) kope_vulkan_##name(arg0, arg1, arg2, arg3)
#define KOPE_G5_CALL5(name, arg0, arg1, arg2, arg3, arg4) kope_vulkan_##name(arg0, arg1, arg2, arg3, arg4)
#define KOPE_G5_CALL6(name, arg0, arg1, arg2, arg3, arg4, arg5) kope_vulkan_##name(arg0, arg1, arg2, arg3, arg4, arg5)
#define KOPE_G5_CALL7(name, arg0, arg1, arg2, arg3, arg4, arg5, arg6) kope_vulkan_##name(arg0, arg1, arg2, arg3, arg4, arg5, arg6)

#endif

#ifdef __cplusplus
}
#endif

#endif
