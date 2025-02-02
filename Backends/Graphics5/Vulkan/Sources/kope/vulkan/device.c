#include "device_functions.h"

#include "vulkanunit.h"

#include <kope/graphics5/device.h>
#include <kope/util/align.h>

#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include <assert.h>
#include <stdlib.h>

void kinc_vulkan_get_instance_extensions(const char **extensions, int *index, int max);
VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

static VkBool32 vkDebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Vulkan ERROR: Code %d : %s", pCallbackData->messageIdNumber, pCallbackData->pMessage);
		kinc_debug_break();
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Vulkan WARNING: Code %d : %s", pCallbackData->messageIdNumber, pCallbackData->pMessage);
	}
	return VK_FALSE;
}

static bool check_extensions(const char **wanted_extensions, int wanted_extension_count, VkExtensionProperties *extensions, int extension_count) {
	bool *found_extensions = calloc(wanted_extension_count, 1);

	for (int i = 0; i < extension_count; i++) {
		for (int i2 = 0; i2 < wanted_extension_count; i2++) {
			if (strcmp(wanted_extensions[i2], extensions[i].extensionName) == 0) {
				found_extensions[i2] = true;
			}
		}
	}

	bool missing_extensions = false;

	for (int i = 0; i < wanted_extension_count; i++) {
		if (!found_extensions[i]) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to find extension %s", wanted_extensions[i]);
			missing_extensions = true;
		}
	}

	free(found_extensions);

	return missing_extensions;
}

struct vk_funs {
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkDestroySurfaceKHR fpDestroySurfaceKHR;

	PFN_vkCreateDebugUtilsMessengerEXT fpCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT fpDestroyDebugUtilsMessengerEXT;

	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
};

static struct vk_funs vk = {0};

#define GET_INSTANCE_PROC_ADDR(instance, entrypoint)                                                                                                           \
	{                                                                                                                                                          \
		vk.fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(instance, "vk" #entrypoint);                                                             \
		if (vk.fp##entrypoint == NULL) {                                                                                                                       \
			kinc_error_message("vkGetInstanceProcAddr failed to find vk" #entrypoint);                                                                         \
		}                                                                                                                                                      \
	}

#ifndef KINC_ANDROID
static VkAllocationCallbacks allocator;
#endif

struct vk_depth {
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
};

#define MAXIMUM_WINDOWS 16

struct vk_window {
	int width;
	int height;

	bool resized;
	bool surface_destroyed;

	int depth_bits;
	int stencil_bits;

	bool vsynced;

	uint32_t current_image;

	VkSurfaceKHR surface;
	VkSurfaceFormatKHR format;

	VkSwapchainKHR swapchain;
	uint32_t image_count;
	VkImage *images;
	VkImageView *views;
	VkFramebuffer *framebuffers;

	VkRenderPass framebuffer_render_pass;
	VkRenderPass rendertarget_render_pass;
	VkRenderPass rendertarget_render_pass_with_depth;

	struct vk_depth depth;
};

struct vk_context {
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memory_properties;

	VkCommandBuffer setup_cmd;
	VkCommandPool cmd_pool;
	VkQueue queue;

	struct vk_window windows[MAXIMUM_WINDOWS];

	// buffer hack
	VkBuffer *vertex_uniform_buffer;
	VkBuffer *fragment_uniform_buffer;
	VkBuffer *compute_uniform_buffer;

	int current_window;

#ifdef VALIDATE
	bool validation_found;
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

#ifndef KINC_ANDROID
static VKAPI_ATTR void *VKAPI_CALL myrealloc(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
#ifdef _MSC_VER
	return _aligned_realloc(pOriginal, size, alignment);
#else
	return realloc(pOriginal, size);
#endif
}

static VKAPI_ATTR void *VKAPI_CALL myalloc(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
#ifdef _MSC_VER
	return _aligned_malloc(size, alignment);
#else
	void *ptr;

	if (alignment % sizeof(void *) != 0) {
		alignment *= (sizeof(void *) / alignment);
	}

	if (posix_memalign(&ptr, alignment, size) != 0) {
		return NULL;
	}
	return ptr;
#endif
}

static VKAPI_ATTR void VKAPI_CALL myfree(void *pUserData, void *pMemory) {
#ifdef _MSC_VER
	_aligned_free(pMemory);
#else
	free(pMemory);
#endif
}
#endif

static struct vk_context vk_ctx = {0};

static VkPhysicalDeviceProperties gpu_props;

static uint32_t queue_count;

static VkQueueFamilyProperties *queue_props;

static uint32_t graphics_queue_node_index;

static VkPhysicalDeviceMemoryProperties memory_properties;

static bool find_layer(VkLayerProperties *layers, int layer_count, const char *wanted_layer) {
	for (int i = 0; i < layer_count; i++) {
		if (strcmp(wanted_layer, layers[i].layerName) == 0) {
			return true;
		}
	}

	return false;
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void kope_vulkan_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
	VkResult err;
	uint32_t instance_layer_count = 0;

	static const char *wanted_instance_layers[64];
	int wanted_instance_layer_count = 0;

	err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
	assert(!err);

	if (instance_layer_count > 0) {
		VkLayerProperties *instance_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
		assert(!err);

#ifdef VALIDATE
		vk_ctx.validation_found = find_layer(instance_layers, instance_layer_count, "VK_LAYER_KHRONOS_validation");
		if (vk_ctx.validation_found) {
			kinc_log(KINC_LOG_LEVEL_INFO, "Running with Vulkan validation layers enabled.");
			wanted_instance_layers[wanted_instance_layer_count++] = "VK_LAYER_KHRONOS_validation";
		}
#endif

		free(instance_layers);
	}

	static const char *wanted_instance_extensions[64];
	int wanted_instance_extension_count = 0;

	uint32_t instance_extension_count = 0;

	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
	kinc_vulkan_get_instance_extensions(wanted_instance_extensions, &wanted_instance_extension_count, ARRAY_SIZE(wanted_instance_extensions));

	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	assert(!err);
	VkExtensionProperties *instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
	assert(!err);
	bool missing_instance_extensions =
	    check_extensions(wanted_instance_extensions, wanted_instance_extension_count, instance_extensions, instance_extension_count);

	if (missing_instance_extensions) {
		kinc_error();
	}

#ifdef VALIDATE
	// this extension should be provided by the validation layers
	if (vk_ctx.validation_found) {
		wanted_instance_extensions[wanted_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}
#endif

	VkApplicationInfo app = {0};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL;
	app.pApplicationName = kinc_application_name();
	app.applicationVersion = 0;
	app.pEngineName = "Kore";
	app.engineVersion = 0;
#ifdef KINC_VKRT
	app.apiVersion = VK_API_VERSION_1_2;
#else
	app.apiVersion = VK_API_VERSION_1_0;
#endif

	VkInstanceCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.pApplicationInfo = &app;
#ifdef VALIDATE
	if (vk_ctx.validation_found) {
		info.enabledLayerCount = wanted_instance_layer_count;
		info.ppEnabledLayerNames = (const char *const *)wanted_instance_layers;
	}
	else
#endif
	{
		info.enabledLayerCount = 0;
		info.ppEnabledLayerNames = NULL;
	}
	info.enabledExtensionCount = wanted_instance_extension_count;
	info.ppEnabledExtensionNames = (const char *const *)wanted_instance_extensions;

#ifndef KINC_ANDROID
	allocator.pfnAllocation = myalloc;
	allocator.pfnFree = myfree;
	allocator.pfnReallocation = myrealloc;
	err = vkCreateInstance(&info, &allocator, &vk_ctx.instance);
#else
	err = vkCreateInstance(&info, NULL, &vk_ctx.instance);
#endif
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
		kinc_error_message("Vulkan driver is incompatible");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
		kinc_error_message("Vulkan extension not found");
	}
	else if (err) {
		kinc_error_message("Can not create Vulkan instance");
	}

	uint32_t gpu_count;

	err = vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, NULL);
	assert(!err && gpu_count > 0);

	bool headless = false;

	// TODO: expose gpu selection to user?
	if (gpu_count > 0) {
		VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, physical_devices);
		assert(!err);
		// The device with the highest score is chosen.
		float best_score = 0.0;
		for (uint32_t gpu_idx = 0; gpu_idx < gpu_count; gpu_idx++) {
			VkPhysicalDevice gpu = physical_devices[gpu_idx];
			uint32_t queue_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);
			VkQueueFamilyProperties *queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);
			bool can_present = false;
			bool can_render = false;
			// According to the documentation, a device that supports graphics must also support compute,
			// Just to be 100% safe verify that it supports both anyway.
			bool can_compute = false;
			for (uint32_t i = 0; i < queue_count; i++) {
				VkBool32 queue_supports_present = kinc_vulkan_get_physical_device_presentation_support(gpu, i);
				if (queue_supports_present) {
					can_present = true;
				}
				VkQueueFamilyProperties queue_properties = queue_props[i];
				uint32_t flags = queue_properties.queueFlags;
				if ((flags & VK_QUEUE_GRAPHICS_BIT) != 0) {
					can_render = true;
				}
				if ((flags & VK_QUEUE_COMPUTE_BIT) != 0) {
					can_compute = true;
				}
			}
			if (!can_present || !can_render || !can_compute) {
				// This device is missing required features so move on
				continue;
			}

			// Score the device in order to compare it to others.
			// Higher score = better.
			float score = 0.0;
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(gpu, &properties);
			switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score += 10;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score += 7;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score += 5;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				score += 1;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				// CPU gets a score of zero
				break;
			case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
				break;
			}
			// TODO: look into using more metrics than just the device type for scoring, eg: available memory, max texture sizes, etc.
			// If this is the first usable device, skip testing against the previous best.
			if (vk_ctx.gpu == VK_NULL_HANDLE || score > best_score) {
				vk_ctx.gpu = gpu;
				best_score = score;
			}
		}
		if (vk_ctx.gpu == VK_NULL_HANDLE) {
			if (headless) {
				vk_ctx.gpu = physical_devices[0];
			}
			else {
				kinc_error_message("No Vulkan device that supports presentation found");
			}
		}
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(vk_ctx.gpu, &properties);
		kinc_log(KINC_LOG_LEVEL_INFO, "Chosen Vulkan device: %s", properties.deviceName);
		free(physical_devices);
	}
	else {
		kinc_error_message("No Vulkan device found");
	}

	static const char *wanted_device_layers[64];
	int wanted_device_layer_count = 0;

	uint32_t device_layer_count = 0;
	err = vkEnumerateDeviceLayerProperties(vk_ctx.gpu, &device_layer_count, NULL);
	assert(!err);

	if (device_layer_count > 0) {
		VkLayerProperties *device_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * device_layer_count);
		err = vkEnumerateDeviceLayerProperties(vk_ctx.gpu, &device_layer_count, device_layers);
		assert(!err);

#ifdef VALIDATE
		vk_ctx.validation_found = find_layer(device_layers, device_layer_count, "VK_LAYER_KHRONOS_validation");
		if (vk_ctx.validation_found) {
			wanted_device_layers[wanted_device_layer_count++] = "VK_LAYER_KHRONOS_validation";
		}
#endif

		free(device_layers);
	}

	const char *wanted_device_extensions[64];
	int wanted_device_extension_count = 0;

	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	// Allows negative viewport height to flip viewport
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;

#ifdef KINC_VKRT
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
#endif

#ifndef VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME // For Dave's Debian
#define VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME "VK_KHR_format_feature_flags2"
#endif

	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME;

	uint32_t device_extension_count = 0;

	err = vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, NULL);
	assert(!err);

	VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
	err = vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, device_extensions);
	assert(!err);

	bool missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);
	if (missing_device_extensions) {
		wanted_device_extension_count -= 1; // remove VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME
	}
	missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);

	free(device_extensions);

	if (missing_device_extensions) {
		exit(1);
	}

#ifdef VALIDATE
	if (vk_ctx.validation_found) {
		GET_INSTANCE_PROC_ADDR(vk_ctx.instance, CreateDebugUtilsMessengerEXT);

		VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo = {0};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dbgCreateInfo.flags = 0;
		dbgCreateInfo.pfnUserCallback = vkDebugUtilsMessengerCallbackEXT;
		dbgCreateInfo.pUserData = NULL;
		dbgCreateInfo.pNext = NULL;
		dbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		dbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		err = vk.fpCreateDebugUtilsMessengerEXT(vk_ctx.instance, &dbgCreateInfo, NULL, &vk_ctx.debug_messenger);
		assert(!err);
	}
#endif

	// Having these GIPA queries of vk_ctx.device extension entry points both
	// BEFORE and AFTER vkCreateDevice is a good test for the loader
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, CreateSwapchainKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, DestroySwapchainKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, DestroySurfaceKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetSwapchainImagesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, AcquireNextImageKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, QueuePresentKHR);

	vkGetPhysicalDeviceProperties(vk_ctx.gpu, &gpu_props);

	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(vk_ctx.gpu, &queue_count, NULL);

	queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(vk_ctx.gpu, &queue_count, queue_props);
	assert(queue_count >= 1);

	if (!headless) {
		// Iterate over each queue to learn whether it supports presenting:
		VkBool32 *supportsPresent = (VkBool32 *)malloc(queue_count * sizeof(VkBool32));
		for (uint32_t i = 0; i < queue_count; i++) {
			supportsPresent[i] = kinc_vulkan_get_physical_device_presentation_support(vk_ctx.gpu, i);
			// vk.fpGetPhysicalDeviceSurfaceSupportKHR(vk_ctx.gpu, i, surface, &supportsPresent[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queue_count; i++) {
			if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				if (graphicsQueueNodeIndex == UINT32_MAX) {
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresent[i] == VK_TRUE) {
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (presentQueueNodeIndex == UINT32_MAX) {
			// If didn't find a queue that supports both graphics and present, then
			// find a separate present queue.
			for (uint32_t i = 0; i < queue_count; ++i) {
				if (supportsPresent[i] == VK_TRUE) {
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		free(supportsPresent);

		// Generate error if could not find both a graphics and a present queue
		if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
			kinc_error_message("Graphics or present queue not found");
		}

		// TODO: Add support for separate queues, including presentation,
		//       synchronization, and appropriate tracking for QueueSubmit.
		// NOTE: While it is possible for an application to use a separate graphics
		//       and a present queues, this demo program assumes it is only using
		//       one:
		if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
			kinc_error_message("Graphics and present queue do not match");
		}

		graphics_queue_node_index = graphicsQueueNodeIndex;

		{
			float queue_priorities[1] = {0.0};
			VkDeviceQueueCreateInfo queue = {0};
			queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue.pNext = NULL;
			queue.queueFamilyIndex = graphics_queue_node_index;
			queue.queueCount = 1;
			queue.pQueuePriorities = queue_priorities;

			VkDeviceCreateInfo deviceinfo = {0};
			deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceinfo.pNext = NULL;
			deviceinfo.queueCreateInfoCount = 1;
			deviceinfo.pQueueCreateInfos = &queue;

			deviceinfo.enabledLayerCount = wanted_device_layer_count;
			deviceinfo.ppEnabledLayerNames = (const char *const *)wanted_device_layers;

			deviceinfo.enabledExtensionCount = wanted_device_extension_count;
			deviceinfo.ppEnabledExtensionNames = (const char *const *)wanted_device_extensions;

#ifdef KINC_VKRT
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineExt = {0};
			rayTracingPipelineExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			rayTracingPipelineExt.pNext = NULL;
			rayTracingPipelineExt.rayTracingPipeline = VK_TRUE;

			VkPhysicalDeviceAccelerationStructureFeaturesKHR rayTracingAccelerationStructureExt = {0};
			rayTracingAccelerationStructureExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			rayTracingAccelerationStructureExt.pNext = &rayTracingPipelineExt;
			rayTracingAccelerationStructureExt.accelerationStructure = VK_TRUE;

			VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressExt = {0};
			bufferDeviceAddressExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			bufferDeviceAddressExt.pNext = &rayTracingAccelerationStructureExt;
			bufferDeviceAddressExt.bufferDeviceAddress = VK_TRUE;

			deviceinfo.pNext = &bufferDeviceAddressExt;
#endif

			err = vkCreateDevice(vk_ctx.gpu, &deviceinfo, NULL, &vk_ctx.device);
			assert(!err);
		}

		vkGetDeviceQueue(vk_ctx.device, graphics_queue_node_index, 0, &vk_ctx.queue);

		vkGetPhysicalDeviceMemoryProperties(vk_ctx.gpu, &memory_properties);

		VkCommandPoolCreateInfo cmd_pool_info = {0};
		cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmd_pool_info.pNext = NULL;
		cmd_pool_info.queueFamilyIndex = graphics_queue_node_index;
		cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		err = vkCreateCommandPool(vk_ctx.device, &cmd_pool_info, NULL, &vk_ctx.cmd_pool);

		assert(!err);
	}
}

void kope_vulkan_device_destroy(kope_g5_device *device) {}

void kope_vulkan_device_set_name(kope_g5_device *device, const char *name) {}

void kope_vulkan_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer) {}

void kope_vulkan_device_create_command_list(kope_g5_device *device, kope_g5_command_list_type type, kope_g5_command_list *list) {}

void kope_vulkan_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture) {}

kope_g5_texture *kope_vulkan_device_get_framebuffer(kope_g5_device *device) {
	return NULL;
}

void kope_vulkan_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list) {}

void kope_vulkan_device_wait_until_idle(kope_g5_device *device) {}

void kope_vulkan_device_create_descriptor_set(kope_g5_device *device, uint32_t descriptor_count, uint32_t dynamic_descriptor_count,
                                              uint32_t bindless_descriptor_count, uint32_t sampler_count, kope_vulkan_descriptor_set *set) {}

void kope_vulkan_device_create_sampler(kope_g5_device *device, const kope_g5_sampler_parameters *parameters, kope_g5_sampler *sampler) {}

void kope_vulkan_device_create_raytracing_volume(kope_g5_device *device, kope_g5_buffer *vertex_buffer, uint64_t vertex_count, kope_g5_buffer *index_buffer,
                                                 uint32_t index_count, kope_g5_raytracing_volume *volume) {}

void kope_vulkan_device_create_raytracing_hierarchy(kope_g5_device *device, kope_g5_raytracing_volume **volumes, kinc_matrix4x4_t *volume_transforms,
                                                    uint32_t volumes_count, kope_g5_raytracing_hierarchy *hierarchy) {}

void kope_vulkan_device_create_query_set(kope_g5_device *device, const kope_g5_query_set_parameters *parameters, kope_g5_query_set *query_set) {}

uint32_t kope_vulkan_device_align_texture_row_bytes(kope_g5_device *device, uint32_t row_bytes) {
	return 0;
}

void kope_vulkan_device_create_fence(kope_g5_device *device, kope_g5_fence *fence) {}

void kope_vulkan_device_signal(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value) {}

void kope_vulkan_device_wait(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value) {}
