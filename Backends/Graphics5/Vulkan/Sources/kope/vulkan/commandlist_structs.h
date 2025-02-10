#ifndef KOPE_VULKAN_COMMANDLIST_STRUCTS_HEADER
#define KOPE_VULKAN_COMMANDLIST_STRUCTS_HEADER

#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kope_vulkan_device;
struct kope_vulkan_texture;
struct kope_vulkan_compute_pipeline;
struct kope_vulkan_ray_pipeline;
struct kope_vulkan_rendery_pipeline;
struct kope_vulkan_descriptor_set;
struct kope_g5_query_set;

typedef struct kope_vulkan_buffer_access {
	int nothing;
} kope_vulkan_buffer_access;

typedef struct kope_vulkan_command_list {
	VkDevice device;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
	VkFence fence;
	bool presenting;
} kope_vulkan_command_list;

#ifdef __cplusplus
}
#endif

#endif
