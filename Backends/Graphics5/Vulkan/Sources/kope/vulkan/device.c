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

static VkInstance vulkan_instance;
static VkPhysicalDevice vulkan_gpu;

#ifdef VALIDATE
static bool validation;
static VkDebugUtilsMessengerEXT debug_utils_messenger;
#else
static const bool validation = false;
#endif

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

static VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types,
                               const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Vulkan ERROR: Code %d : %s", callback_data->messageIdNumber, callback_data->pMessage);
		kinc_debug_break();
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Vulkan WARNING: Code %d : %s", callback_data->messageIdNumber, callback_data->pMessage);
	}
	return VK_FALSE;
}

#ifndef KINC_ANDROID
static VKAPI_ATTR void *VKAPI_CALL vulkan_realloc(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
#ifdef _MSC_VER
	return _aligned_realloc(pOriginal, size, alignment);
#else
	return realloc(pOriginal, size);
#endif
}

static VKAPI_ATTR void *VKAPI_CALL vulkan_alloc(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
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

static VKAPI_ATTR void VKAPI_CALL vulkan_free(void *pUserData, void *pMemory) {
#ifdef _MSC_VER
	_aligned_free(pMemory);
#else
	free(pMemory);
#endif
}
#endif

static bool check_extensions(const char **extensions, int extensions_count, VkExtensionProperties *extension_properties, int extension_properties_count) {
	for (int extension_index = 0; extension_index < extensions_count; ++extension_index) {
		bool found = false;

		for (int extension_property_index = 0; extension_property_index < extension_properties_count; ++extension_property_index) {
			if (strcmp(extensions[extension_index], extension_properties[extension_property_index].extensionName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Failed to find extension %s", extensions[extension_index]);
			return false;
		}
	}

	return true;
}

static bool check_instance_extensions(const char **instance_extensions, int instance_extensions_count) {
	VkExtensionProperties instance_extension_properties[256];
	uint32_t instance_extension_properties_count = sizeof(instance_extension_properties) / sizeof(instance_extension_properties[0]);

	VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_properties_count, instance_extension_properties);
	assert(result == VK_SUCCESS);

	return check_extensions(instance_extensions, instance_extensions_count, instance_extension_properties, instance_extension_properties_count);
}

static bool check_device_extensions(const char **device_extensions, int device_extensions_count) {
	VkExtensionProperties device_extension_properties[256];
	uint32_t device_extension_properties_count = sizeof(device_extension_properties) / sizeof(device_extension_properties[0]);

	VkResult result = vkEnumerateDeviceExtensionProperties(vulkan_gpu, NULL, &device_extension_properties_count, device_extension_properties);
	assert(result == VK_SUCCESS);

	return check_extensions(device_extensions, device_extensions_count, device_extension_properties, device_extension_properties_count);
}

static bool check_layers(const char **layers, int layers_count, VkLayerProperties *layer_properties, int layer_properties_count) {
	for (int layer_index = 0; layer_index < layers_count; ++layer_index) {
		bool found = false;

		for (int layer_property_index = 0; layer_property_index < layer_properties_count; ++layer_property_index) {
			if (strcmp(layers[layer_index], layer_properties[layer_property_index].layerName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Failed to find extension %s", layers[layer_index]);
			return false;
		}
	}

	return true;
}

static bool check_instance_layers(const char **instance_layers, int instance_layers_count) {
	VkLayerProperties instance_layer_properties[256];
	uint32_t instance_layer_properties_count = sizeof(instance_layer_properties) / sizeof(instance_layer_properties[0]);

	VkResult result = vkEnumerateInstanceLayerProperties(&instance_layer_properties_count, instance_layer_properties);
	assert(result == VK_SUCCESS);

	return check_layers(instance_layers, instance_layers_count, instance_layer_properties, instance_layer_properties_count);
}

static bool check_device_layers(const char **device_layers, int device_layers_count) {
	VkLayerProperties device_layer_properties[256];
	uint32_t device_layer_properties_count = sizeof(device_layer_properties) / sizeof(device_layer_properties[0]);

	VkResult result = vkEnumerateDeviceLayerProperties(vulkan_gpu, &device_layer_properties_count, device_layer_properties);
	assert(result == VK_SUCCESS);

	return check_layers(device_layers, device_layers_count, device_layer_properties, device_layer_properties_count);
}

static void load_extension_functions(void) {
#define GET_VULKAN_FUNCTION(entrypoint)                                                                                                                        \
	{                                                                                                                                                          \
		vulkan_##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(vulkan_instance, "vk" #entrypoint);                                                    \
		if (vulkan_##entrypoint == NULL) {                                                                                                                     \
			kinc_error_message("vkGetInstanceProcAddr failed to find vk" #entrypoint);                                                                         \
		}                                                                                                                                                      \
	}

	if (validation) {
		GET_VULKAN_FUNCTION(CreateDebugUtilsMessengerEXT);
	}

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

uint32_t find_graphics_queue_family(void) {
	VkQueueFamilyProperties queue_family_props[16];
	uint32_t queue_family_count = sizeof(queue_family_props) / sizeof(queue_family_props[0]);

	vkGetPhysicalDeviceQueueFamilyProperties(vulkan_gpu, &queue_family_count, queue_family_props);

	for (uint32_t queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index) {
		if ((queue_family_props[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
		    vkGetPhysicalDeviceWin32PresentationSupportKHR(vulkan_gpu, queue_family_index)) {
			return queue_family_index;
		}
	}

	kinc_error_message("Graphics or present queue not found");
	return 0;
}

void kope_vulkan_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
	const char *instance_layers[64];
	int instance_layers_count = 0;

#ifdef VALIDATE
	instance_layers[instance_layers_count++] = "VK_LAYER_KHRONOS_validation";
#endif

	if (check_instance_layers(instance_layers, instance_layers_count)) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Running with Vulkan validation layers enabled.");
	}
	else {
		--instance_layers_count; // Remove VK_LAYER_KHRONOS_validation
	}

	const char *instance_extensions[64];
	int instance_extensions_count = 0;

	instance_extensions[instance_extensions_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
	instance_extensions[instance_extensions_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#ifdef KINC_WINDOWS
	instance_extensions[instance_extensions_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif

	check_instance_extensions(instance_extensions, instance_extensions_count);

	if (validation) {
		instance_extensions[instance_extensions_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

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

	info.enabledLayerCount = instance_layers_count;
	info.ppEnabledLayerNames = (const char *const *)instance_layers;

	info.enabledExtensionCount = instance_extensions_count;
	info.ppEnabledExtensionNames = (const char *const *)instance_extensions;

#ifndef KINC_ANDROID
	VkAllocationCallbacks allocator;
	allocator.pfnAllocation = vulkan_alloc;
	allocator.pfnFree = vulkan_free;
	allocator.pfnReallocation = vulkan_realloc;
	VkResult result = vkCreateInstance(&info, &allocator, &vulkan_instance);
#else
	VkResult result = vkCreateInstance(&info, NULL, &vulkan_instance);
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

	const char *device_layers[64];
	int device_layers_count = 0;

	device_layers[device_layers_count++] = "VK_LAYER_KHRONOS_validation";

#ifdef VALIDATE
	if (check_device_layers(device_layers, device_layers_count)) {
		validation |= true;
	}
	else {
		--device_layers_count; // Remove VK_LAYER_KHRONOS_validation
	}
#endif

	const char *device_extensions[64];
	int device_extension_count = 0;

	device_extensions[device_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	// Allows negative viewport height to flip viewport
	device_extensions[device_extension_count++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;

#ifdef KINC_VKRT
	device_extensions[device_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
	device_extensions[device_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
	device_extensions[device_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
	device_extensions[device_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
	device_extensions[device_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	device_extensions[device_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
	device_extensions[device_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
#endif

#ifndef VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME // For Dave's Debian
#define VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME "VK_KHR_format_feature_flags2"
#endif

	device_extensions[device_extension_count++] = VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME;

	VkExtensionProperties device_extension_properties[256];
	uint32_t device_extension_properties_count = sizeof(device_extension_properties) / sizeof(device_extension_properties[0]);
	result = vkEnumerateDeviceExtensionProperties(vulkan_gpu, NULL, &device_extension_properties_count, device_extension_properties);
	assert(result == VK_SUCCESS);

	if (!check_extensions(device_extensions, device_extension_count, device_extension_properties, device_extension_properties_count)) {
		device_extension_count -= 1; // remove VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME
	}

	if (!check_extensions(device_extensions, device_extension_count, device_extension_properties, device_extension_properties_count)) {
		kinc_error_message("Missing device extensions");
	}

	if (validation) {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.flags = 0;
		create_info.pfnUserCallback = debug_callback;
		create_info.pUserData = NULL;
		create_info.pNext = NULL;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		result = vulkan_CreateDebugUtilsMessengerEXT(vulkan_instance, &create_info, NULL, &debug_utils_messenger);
		assert(result == VK_SUCCESS);
	}

	uint32_t graphics_queue_family_index = find_graphics_queue_family();

	float queue_priorities[1] = {0.0};
	VkDeviceQueueCreateInfo queue_create_info = {0};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.pNext = NULL;
	queue_create_info.queueFamilyIndex = graphics_queue_family_index;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo device_create_info = {0};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = NULL;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &queue_create_info;

	device_create_info.enabledLayerCount = device_layers_count;
	device_create_info.ppEnabledLayerNames = (const char *const *)device_layers;

	device_create_info.enabledExtensionCount = device_extension_count;
	device_create_info.ppEnabledExtensionNames = (const char *const *)device_extensions;

#ifdef KINC_VKRT
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_pipeline = {0};
	raytracing_pipeline.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	raytracing_pipeline.pNext = NULL;
	raytracing_pipeline.rayTracingPipeline = VK_TRUE;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR raytracing_acceleration_structure = {0};
	raytracing_acceleration_structure.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	raytracing_acceleration_structure.pNext = &raytracing_pipeline;
	raytracing_acceleration_structure.accelerationStructure = VK_TRUE;

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address = {0};
	buffer_device_address.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	buffer_device_address.pNext = &raytracing_acceleration_structure;
	buffer_device_address.bufferDeviceAddress = VK_TRUE;

	device_create_info.pNext = &buffer_device_address;
#endif

	result = vkCreateDevice(vulkan_gpu, &device_create_info, NULL, &device->vulkan.device);
	assert(result == VK_SUCCESS);

	vkGetDeviceQueue(device->vulkan.device, graphics_queue_family_index, 0, &device->vulkan.queue);

	vkGetPhysicalDeviceMemoryProperties(vulkan_gpu, &device->vulkan.device_memory_properties);

	VkCommandPoolCreateInfo command_pool_create_info = {0};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext = NULL;
	command_pool_create_info.queueFamilyIndex = graphics_queue_family_index;
	command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	result = vkCreateCommandPool(device->vulkan.device, &command_pool_create_info, NULL, &device->vulkan.command_pool);
	assert(result == VK_SUCCESS);
}

void kope_vulkan_device_destroy(kope_g5_device *device) {}

void kope_vulkan_device_set_name(kope_g5_device *device, const char *name) {}

static bool memory_type_from_properties(kope_g5_device *device, uint32_t type_bits, VkFlags requirements_mask, uint32_t *type_index) {
	for (uint32_t i = 0; i < 32; ++i) {
		if ((type_bits & 1) == 1) {
			if ((device->vulkan.device_memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*type_index = i;
				return true;
			}
		}
		type_bits >>= 1;
	}
	return false;
}

void kope_vulkan_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer) {
	buffer->vulkan.device = device->vulkan.device;

	buffer->vulkan.size = parameters->size;

	VkBufferCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.size = parameters->size;
	create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
#ifdef KINC_VKRT
	create_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
#endif
	create_info.flags = 0;

	VkResult result = vkCreateBuffer(device->vulkan.device, &create_info, NULL, &buffer->vulkan.buffer);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memory_requirements = {0};
	vkGetBufferMemoryRequirements(device->vulkan.device, buffer->vulkan.buffer, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info;
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.pNext = NULL;
	memory_allocate_info.memoryTypeIndex = 0;
	memory_allocate_info.allocationSize = memory_requirements.size;
	bool memory_type_found =
	    memory_type_from_properties(device, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memory_allocate_info.memoryTypeIndex);
	assert(memory_type_found);

#ifdef KINC_VKRT
	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	memory_allocate_info.pNext = &memory_allocate_flags_info;
#endif

	result = vkAllocateMemory(device->vulkan.device, &memory_allocate_info, NULL, &buffer->vulkan.memory);
	assert(result == VK_SUCCESS);

	result = vkBindBufferMemory(device->vulkan.device, buffer->vulkan.buffer, buffer->vulkan.memory, 0);
	assert(result == VK_SUCCESS);
}

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
