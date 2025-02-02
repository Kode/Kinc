#ifndef KOPE_WEBGPU_COMMANDLIST_STRUCTS_HEADER
#define KOPE_WEBGPU_COMMANDLIST_STRUCTS_HEADER

#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kope_webgpu_device;
struct kope_webgpu_texture;
struct kope_webgpu_compute_pipeline;
struct kope_webgpu_ray_pipeline;
struct kope_webgpu_rendery_pipeline;
struct kope_webgpu_descriptor_set;
struct kope_g5_query_set;

typedef struct kope_webgpu_buffer_access {
	int nothing;
} kope_webgpu_buffer_access;

typedef struct kope_webgpu_command_list {
	int nothing;
} kope_webgpu_command_list;

#ifdef __cplusplus
}
#endif

#endif
