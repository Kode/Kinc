#ifndef KOPE_WEBGPU_DEVICE_STRUCTS_HEADER
#define KOPE_WEBGPU_DEVICE_STRUCTS_HEADER

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/texture.h>
#include <kope/util/indexallocator.h>
#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KOPE_WEBGPU_FRAME_COUNT 2

typedef struct kope_webgpu_device {
	int nothing;
} kope_webgpu_device;

typedef struct kope_webgpu_query_set {
	int nothing;
} kope_webgpu_query_set;

typedef struct kope_webgpu_raytracing_volume {
	int nothing;
} kope_webgpu_raytracing_volume;

typedef struct kope_webgpu_raytracing_hierarchy {
	int nothing;
} kope_webgpu_raytracing_hierarchy;

#ifdef __cplusplus
}
#endif

#endif
