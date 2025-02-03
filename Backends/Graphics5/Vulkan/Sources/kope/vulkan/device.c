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

static void get_instance_extensions(const char **names, int *index, int max) {
	assert(*index + 1 < max);
	names[(*index)++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

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

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR vulkan_GetPhysicalDeviceSurfaceSupportKHR = NULL;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vulkan_GetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vulkan_GetPhysicalDeviceSurfaceFormatsKHR = NULL;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vulkan_GetPhysicalDeviceSurfacePresentModesKHR = NULL;
static PFN_vkCreateSwapchainKHR vulkan_CreateSwapchainKHR = NULL;
static PFN_vkDestroySwapchainKHR vulkan_DestroySwapchainKHR = NULL;
static PFN_vkGetSwapchainImagesKHR vulkan_GetSwapchainImagesKHR = NULL;
static PFN_vkDestroySurfaceKHR vulkan_DestroySurfaceKHR = NULL;

static PFN_vkCreateDebugUtilsMessengerEXT vulkan_CreateDebugUtilsMessengerEXT = NULL;
static PFN_vkDestroyDebugUtilsMessengerEXT vulkan_DestroyDebugUtilsMessengerEXT = NULL;

static PFN_vkAcquireNextImageKHR vulkan_AcquireNextImageKHR = NULL;
static PFN_vkQueuePresentKHR vulkan_QueuePresentKHR = NULL;

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

static VkInstance vulkan_instance;
static VkPhysicalDevice vulkan_gpu;

#ifdef VALIDATE
static bool validation_found;
static VkDebugUtilsMessengerEXT debug_utils_messenger;
#endif

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

static uint32_t queue_count;

static uint32_t graphics_queue_index;

static bool find_layer(VkLayerProperties *layers, int layer_count, const char *wanted_layer) {
	for (int i = 0; i < layer_count; i++) {
		if (strcmp(wanted_layer, layers[i].layerName) == 0) {
			return true;
		}
	}

	return false;
}

static void load_extension_functions(void) {
#define GET_VULKAN_FUNCTION(entrypoint)                                                                                                                        \
	{                                                                                                                                                          \
		vulkan_##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(vulkan_instance, "vk" #entrypoint);                                                    \
		if (vulkan_##entrypoint == NULL) {                                                                                                                     \
			kinc_error_message("vkGetInstanceProcAddr failed to find vk" #entrypoint);                                                                         \
		}                                                                                                                                                      \
	}

#ifdef VALIDATE
	if (validation_found) {
		GET_VULKAN_FUNCTION(CreateDebugUtilsMessengerEXT);
	}
#endif

	GET_VULKAN_FUNCTION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_VULKAN_FUNCTION(GetPhysicalDeviceSurfaceFormatsKHR);
	GET_VULKAN_FUNCTION(GetPhysicalDeviceSurfacePresentModesKHR);
	GET_VULKAN_FUNCTION(GetPhysicalDeviceSurfaceSupportKHR);
	GET_VULKAN_FUNCTION(CreateSwapchainKHR);
	GET_VULKAN_FUNCTION(DestroySwapchainKHR);
	GET_VULKAN_FUNCTION(DestroySurfaceKHR);
	GET_VULKAN_FUNCTION(GetSwapchainImagesKHR);
	GET_VULKAN_FUNCTION(AcquireNextImageKHR);
	GET_VULKAN_FUNCTION(QueuePresentKHR);

#undef GET_VULKAN_FUNCTION
}

void find_gpu(void) {
	VkPhysicalDevice physical_devices[64];
	uint32_t gpu_count = sizeof(physical_devices) / sizeof(physical_devices[0]);

	VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &gpu_count, physical_devices);

	if (result != VK_SUCCESS || gpu_count == 0) {
		kinc_error_message("No Vulkan device found");
		return;
	}

	float best_score = -1.0;

	for (uint32_t gpu_index = 0; gpu_index < gpu_count; ++gpu_index) {
		VkPhysicalDevice gpu = physical_devices[gpu_index];

		VkQueueFamilyProperties queue_props[64];
		uint32_t queue_count = sizeof(queue_props) / sizeof(queue_props[0]);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);

		bool can_present = false;
		bool can_render = false;

		for (uint32_t queue_index = 0; queue_index < queue_count; ++queue_index) {
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu, queue_index)) {
				can_present = true;
			}

			if ((queue_props[queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				can_render = true;
			}
		}

		if (!can_present || !can_render) {
			continue;
		}

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
			break;
		case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
			break;
		}

		if (score > best_score) {
			vulkan_gpu = gpu;
			best_score = score;
		}
	}

	if (vulkan_gpu == VK_NULL_HANDLE) {
		kinc_error_message("No Vulkan device that supports presentation found");
		return;
	}

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(vulkan_gpu, &properties);
	kinc_log(KINC_LOG_LEVEL_INFO, "Chosen Vulkan device: %s", properties.deviceName);
}

void find_queue(void) {
	VkQueueFamilyProperties queue_props[16];
	queue_count = sizeof(queue_props) / sizeof(queue_props[0]);

	vkGetPhysicalDeviceQueueFamilyProperties(vulkan_gpu, &queue_count, queue_props);
	assert(queue_count >= 1);

	for (uint32_t queue_index = 0; queue_index < queue_count; ++queue_index) {
		if ((queue_props[queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && vkGetPhysicalDeviceWin32PresentationSupportKHR(vulkan_gpu, queue_index)) {
			graphics_queue_index = queue_index;
			return;
		}
	}

	kinc_error_message("Graphics or present queue not found");
}

void kope_vulkan_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
	const char *wanted_instance_layers[64];
	int wanted_instance_layer_count = 0;

	VkLayerProperties instance_layers[256];
	uint32_t instance_layer_count = sizeof(instance_layers) / sizeof(instance_layers[0]);

	VkResult result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
	assert(result == VK_SUCCESS);

#ifdef VALIDATE
	validation_found = find_layer(instance_layers, instance_layer_count, "VK_LAYER_KHRONOS_validation");
	if (validation_found) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Running with Vulkan validation layers enabled.");
		wanted_instance_layers[wanted_instance_layer_count++] = "VK_LAYER_KHRONOS_validation";
	}
#endif

	free(instance_layers);

	const char *wanted_instance_extensions[64];
	int wanted_instance_extension_count = 0;

	uint32_t instance_extension_count = 0;

	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
	get_instance_extensions(wanted_instance_extensions, &wanted_instance_extension_count,
	                        sizeof(wanted_instance_extensions) / sizeof(wanted_instance_extensions[0]));

	result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	assert(result == VK_SUCCESS);
	VkExtensionProperties *instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
	result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
	assert(result == VK_SUCCESS);
	bool missing_instance_extensions =
	    check_extensions(wanted_instance_extensions, wanted_instance_extension_count, instance_extensions, instance_extension_count);

	if (missing_instance_extensions) {
		kinc_error();
	}

#ifdef VALIDATE
	// this extension should be provided by the validation layers
	if (validation_found) {
		wanted_instance_extensions[wanted_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}
#endif

	VkApplicationInfo app = {0};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL;
	app.pApplicationName = kinc_application_name();
	app.applicationVersion = 0;
	app.pEngineName = "Kope";
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
	if (validation_found) {
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
	result = vkCreateInstance(&info, &allocator, &vulkan_instance);
#else
	result = vkCreateInstance(&info, NULL, &vulkan_instance);
#endif
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		kinc_error_message("Vulkan driver is incompatible");
	}
	else if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		kinc_error_message("Vulkan extension not found");
	}
	else if (result != VK_SUCCESS) {
		kinc_error_message("Can not create Vulkan instance");
	}

	find_gpu();

	static const char *wanted_device_layers[64];
	int wanted_device_layer_count = 0;

	uint32_t device_layer_count = 0;
	result = vkEnumerateDeviceLayerProperties(vulkan_gpu, &device_layer_count, NULL);
	assert(result == VK_SUCCESS);

	if (device_layer_count > 0) {
		VkLayerProperties *device_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * device_layer_count);
		result = vkEnumerateDeviceLayerProperties(vulkan_gpu, &device_layer_count, device_layers);
		assert(result == VK_SUCCESS);

#ifdef VALIDATE
		validation_found = find_layer(device_layers, device_layer_count, "VK_LAYER_KHRONOS_validation");
		if (validation_found) {
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

	result = vkEnumerateDeviceExtensionProperties(vulkan_gpu, NULL, &device_extension_count, NULL);
	assert(result == VK_SUCCESS);

	VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
	result = vkEnumerateDeviceExtensionProperties(vulkan_gpu, NULL, &device_extension_count, device_extensions);
	assert(result == VK_SUCCESS);

	bool missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);
	if (missing_device_extensions) {
		wanted_device_extension_count -= 1; // remove VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME
	}
	missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);

	free(device_extensions);

	if (missing_device_extensions) {
		kinc_error_message("Missing device extensions");
	}

#ifdef VALIDATE
	if (validation_found) {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.flags = 0;
		create_info.pfnUserCallback = vkDebugUtilsMessengerCallbackEXT;
		create_info.pUserData = NULL;
		create_info.pNext = NULL;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		result = vulkan_CreateDebugUtilsMessengerEXT(vulkan_instance, &create_info, NULL, &debug_utils_messenger);
		assert(result == VK_SUCCESS);
	}
#endif

	find_queue();

	{
		float queue_priorities[1] = {0.0};
		VkDeviceQueueCreateInfo queue = {0};
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext = NULL;
		queue.queueFamilyIndex = graphics_queue_index;
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

		result = vkCreateDevice(vulkan_gpu, &deviceinfo, NULL, &device->vulkan.device);
		assert(result == VK_SUCCESS);
	}

	vkGetDeviceQueue(device->vulkan.device, graphics_queue_index, 0, &device->vulkan.queue);

	vkGetPhysicalDeviceMemoryProperties(vulkan_gpu, &device->vulkan.device_memory_properties);

	VkCommandPoolCreateInfo cmd_pool_info = {0};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = graphics_queue_index;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	result = vkCreateCommandPool(device->vulkan.device, &cmd_pool_info, NULL, &device->vulkan.command_pool);
	assert(result == VK_SUCCESS);
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
