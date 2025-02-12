#ifndef KOPE_METAL_COMMANDLIST_STRUCTS_HEADER
#define KOPE_METAL_COMMANDLIST_STRUCTS_HEADER

#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kope_metal_device;
struct kope_metal_texture;
struct kope_metal_compute_pipeline;
struct kope_metal_ray_pipeline;
struct kope_metal_rendery_pipeline;
struct kope_metal_descriptor_set;
struct kope_g5_query_set;

typedef struct kope_metal_buffer_access {
	int nothing;
} kope_metal_buffer_access;

typedef struct kope_metal_command_list {
	void *command_queue;
	void *command_buffer;
	void *render_command_encoder;
	void *index_buffer;
	bool sixteen_bit_indices;
} kope_metal_command_list;

#ifdef __cplusplus
}
#endif

#endif
