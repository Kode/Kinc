#pragma once

#include <kinc/log.h>
#include <vulkan/vulkan.h>
#define MAX_WINDOW_COUNT 16

struct vulkan_context {
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;

	uint32_t queueIndex;
	VkQueue queue;

	VkCommandPool command_pool;
	VkPipelineCache pipeline_cache;
	VkDescriptorPool descriptor_pool;

	VkDebugUtilsMessengerEXT debug_messenger;
};
extern struct vulkan_context context;

#ifndef _NDEBUG
#ifdef _WIN32
#define debug_break() __debugbreak()
#elif __has_builtin(__builtin_trap)
#define debug_break() __builtin_trap()
#else
#define debug_break()
#endif
#else
#define debug_break()
#endif

#define CHECK(r)                                                                                                                                               \
	{                                                                                                                                                          \
		VkResult result = r;                                                                                                                                   \
		if (result) {                                                                                                                                          \
			debug_break();                                                                                                                                     \
			kinc_log(KINC_LOG_LEVEL_ERROR, "Vulkan : Error %x", result);                                                                                       \
			abort();                                                                                                                                           \
		}                                                                                                                                                      \
	}

#define ERROR(message)                                                                                                                                         \
	do {                                                                                                                                                       \
		debug_break();                                                                                                                                         \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Vulkan : Error " message);                                                                                             \
		abort();                                                                                                                                               \
	} while (0)