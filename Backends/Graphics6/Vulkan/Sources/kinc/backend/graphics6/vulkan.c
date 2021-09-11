// TODO :
// Handle window resizing and minimization at the kinc level
// Framebuffer and Renderpass cache, so as to not allocate them every frame.
// GPU memory allocator
// All the other unimplemented stuff

#include "vulkan.h"

#include <kinc/graphics6/commandbuffer.h>
#include <kinc/graphics6/graphics.h>
#include <kinc/graphics6/texture.h>

#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/system.h>

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

// #ifndef _NDEBUG
#define VALIDATE
// #endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#define KINC_SURFACE_EXT_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
#define KINC_SURFACE_EXT_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define KINC_SURFACE_EXT_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif
extern VkResult kinc_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface);

struct vulkan_context context;

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
	kinc_log_level_t log_level = -1;
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		log_level = KINC_LOG_LEVEL_INFO;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		log_level = KINC_LOG_LEVEL_WARNING;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		log_level = KINC_LOG_LEVEL_ERROR;
	}
	if (log_level > -1) {
		kinc_log(log_level, "%s\n", pCallbackData->pMessage);
	}
	return VK_FALSE;
}

void kinc_g6_init() {
	const char *wanted_instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, KINC_SURFACE_EXT_NAME,
#ifdef VALIDATE
	                                            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	const char *wanted_instance_layers[] = {
#ifdef VALIDATE
	    "VK_LAYER_KHRONOS_validation"
#endif
	};

	{
		uint32_t instance_layer_count;
		CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL));
		VkLayerProperties *instance_layers = malloc(sizeof(VkLayerProperties) * instance_layer_count);
		CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers));
		uint32_t wanted_layer_count = sizeof(wanted_instance_layers) / sizeof(wanted_instance_layers[0]);
		uint32_t found_layer_count = 0;

		for (int a = 0; a < instance_layer_count; a++) {
			for (int b = 0; b < wanted_layer_count; b++) {
				if (strcmp(instance_layers[a].layerName, wanted_instance_layers[b]) == 0) {
					found_layer_count++;
				}
			}
		}
		free(instance_layers);
		if (found_layer_count < wanted_layer_count) {
			ERROR("Failed to find the required instance layers.");
		}
	}
	{
		uint32_t instance_extension_count;
		CHECK(vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL));
		VkExtensionProperties *instance_extensions = malloc(sizeof(VkExtensionProperties) * instance_extension_count);
		CHECK(vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions));
		uint32_t wanted_extension_count = sizeof(wanted_instance_extensions) / sizeof(wanted_instance_extensions[0]);
		uint32_t found_extension_count = 0;

		for (int a = 0; a < instance_extension_count; a++) {
			for (int b = 0; b < wanted_extension_count; b++) {
				if (strcmp(instance_extensions[a].extensionName, wanted_instance_extensions[b]) == 0) {
					found_extension_count++;
				}
			}
		}
		free(instance_extensions);
		if (found_extension_count < wanted_extension_count) {
			ERROR("Failed to find the required instance extensions.");
		}
	}

	{
		VkApplicationInfo app = {0};
		app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app.pNext = NULL;
		app.pApplicationName = kinc_application_name();
		app.applicationVersion = 0;
		app.pEngineName = "Kinc";
		app.engineVersion = 0;
		app.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo info = {0};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = NULL;
		info.pApplicationInfo = &app;
		info.enabledLayerCount = sizeof(wanted_instance_layers) / sizeof(wanted_instance_layers[0]);
		info.ppEnabledLayerNames = wanted_instance_layers;
		info.enabledExtensionCount = sizeof(wanted_instance_extensions) / sizeof(wanted_instance_extensions[0]);
		info.ppEnabledExtensionNames = wanted_instance_extensions;

#ifdef VALIDATE
		VkDebugUtilsMessengerCreateInfoEXT debugInfo = {0};
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType =
		    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		debugInfo.pfnUserCallback = debug_callback;
		info.pNext = &debugInfo;
#endif
		CHECK(vkCreateInstance(&info, NULL, &context.instance));
	}
	{
#ifdef VALIDATE
		PFN_vkCreateDebugUtilsMessengerEXT createMessenger = vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
		VkDebugUtilsMessengerCreateInfoEXT info = {0};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity =
		    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType =
		    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = debug_callback;
		CHECK(createMessenger(context.instance, &info, NULL, &context.debug_messenger));
#endif
	}
	{
		uint32_t gpu_count = 0;
		CHECK(vkEnumeratePhysicalDevices(context.instance, &gpu_count, NULL));
		if (gpu_count < 1) ERROR("No gpus found on this system.");
		VkPhysicalDevice *gpus = malloc(sizeof(VkPhysicalDevice) * gpu_count);
		CHECK(vkEnumeratePhysicalDevices(context.instance, &gpu_count, gpus));
		for (int i = 0; i < gpu_count; i++) {
			uint32_t queue_family_property_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_property_count, NULL);
			VkQueueFamilyProperties *queue_familiy_properties = malloc(sizeof(VkQueueFamilyProperties) * queue_family_property_count);
			vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_property_count, queue_familiy_properties);

			bool found = false;
			for (int q = 0; q < queue_family_property_count; q++) {
				if (queue_familiy_properties[0].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					context.gpu = gpus[i];
					context.queueIndex = q;
					found = true;
					break;
				}
			}
			if (found) break;
		}
		free(gpus);
	}
	{
		VkDeviceQueueCreateInfo queue_info = {0};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.pNext = NULL;
		queue_info.queueCount = 1;
		queue_info.queueFamilyIndex = context.queueIndex;
		queue_info.pQueuePriorities = (float[]){1.0f};

		VkDeviceCreateInfo device_info = {0};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext = NULL;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &queue_info;
		device_info.enabledLayerCount = 1;
		device_info.ppEnabledLayerNames = (const char *[]){"VK_LAYER_KHRONOS_validation"};
		device_info.enabledExtensionCount = 1;
		device_info.ppEnabledExtensionNames = (const char *[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		CHECK(vkCreateDevice(context.gpu, &device_info, NULL, &context.device));

		vkGetDeviceQueue(context.device, context.queueIndex, 0, &context.queue);
	}

	{
		VkCommandPoolCreateInfo create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.queueFamilyIndex = context.queueIndex;
		create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		// TODO make one command pool per thread
		CHECK(vkCreateCommandPool(context.device, &create_info, NULL, &context.command_pool));
	}

	{
		VkPipelineCacheCreateInfo create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.flags = 0;
		// TODO cache pipeline data between runs?
		create_info.initialDataSize = 0;
		create_info.pInitialData = NULL;

		CHECK(vkCreatePipelineCache(context.device, &create_info, NULL, &context.pipeline_cache));
	}
	{
		// TODO: Should this use some form of custom allocator
		VkDescriptorPoolCreateInfo create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pNext = 0;
		create_info.flags = 0;
		create_info.maxSets = 64 * 7;
		create_info.poolSizeCount = 7;
		create_info.pPoolSizes = (VkDescriptorPoolSize[7]){(VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 64},
		                                                   (VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 64},
		                                                   (VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 64},
		                                                   (VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 64},
		                                                   (VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 64},
		                                                   (VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 64},
		                                                   (VkDescriptorPoolSize){.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount = 64}};
		vkCreateDescriptorPool(context.device, &create_info, NULL, &context.descriptor_pool);
	}
	kinc_vulkan_allocator_init();
}
void kinc_g6_destroy() {
	kinc_vulkan_allocator_destroy();
	vkDestroyPipelineCache(context.device, context.pipeline_cache, NULL);
	vkDestroyCommandPool(context.device, context.command_pool, NULL);
	vkDestroyDevice(context.device, NULL);
#ifdef VALIDATE
	PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger = vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
	destroyMessenger(context.instance, context.debug_messenger, NULL);
#endif
	vkDestroyInstance(context.instance, NULL);
}

void create_swapchain(kinc_g6_swapchain_t *swapchain, int width, int height) {
	VkSwapchainKHR old_swapchain = swapchain->impl.swapchain;
	VkSurfaceKHR surface = swapchain->impl.surface;

	{
		VkBool32 supported = VK_FALSE;
		CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(context.gpu, context.queueIndex, surface, &supported));
		if (supported != VK_TRUE) {
			ERROR("Surface not supported by gpu.");
		}
	}

	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t format_count = 0;
	VkSurfaceFormatKHR *formats;
	uint32_t present_mode_count = 0;
	VkPresentModeKHR *present_modes;

	CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.gpu, surface, &capabilities));

	CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(context.gpu, surface, &format_count, NULL));
	formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
	CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(context.gpu, surface, &format_count, formats));

	CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(context.gpu, surface, &present_mode_count, NULL));
	present_modes = malloc(sizeof(VkPresentModeKHR) * present_mode_count);
	CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(context.gpu, surface, &present_mode_count, present_modes));

	VkSurfaceFormatKHR format = formats[0];

	for (int i = 0; i < format_count; i++) {
		if (formats[i].format == VK_FORMAT_B8G8R8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = formats[i];
			break;
		}
	}

	if (format.format == VK_FORMAT_UNDEFINED) {
		format.format = VK_FORMAT_B8G8R8A8_SRGB;
		format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	free(formats);

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	for (int i = 0; i < present_mode_count; i++) {
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = present_modes[i];
			break;
		}
	}
	free(present_modes);

	// TODO check with min and max width
	VkExtent2D extent = {width, height};

	uint32_t image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = 0;
	create_info.surface = surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = format.format;
	create_info.imageColorSpace = format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = NULL;
	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	create_info.oldSwapchain = old_swapchain;

	CHECK(vkCreateSwapchainKHR(context.device, &create_info, NULL, &(swapchain->impl.swapchain)));
	if (old_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(context.device, old_swapchain, NULL);
	}
	swapchain->impl.format = format.format;
	swapchain->impl.color_space = format.colorSpace;
	swapchain->impl.extent = extent;
	// if (swapchain->impl.views != NULL) {
	// 	for (int i = 0; i < swapchain->impl.swapchain_image_count; i++) {
	// 		vkDestroyImageView(context.device, swapchain->impl.views[i], NULL);
	// 	}
	// 	free(swapchain->impl.views);
	// }
	if (swapchain->impl.images != NULL) {
		free(swapchain->impl.images);
	}

	CHECK(vkGetSwapchainImagesKHR(context.device, swapchain->impl.swapchain, &swapchain->impl.swapchain_image_count, NULL));
	swapchain->impl.images = malloc(sizeof(VkImage) * swapchain->impl.swapchain_image_count);
	// swapchain->impl.views = malloc(sizeof(VkImageView) * swapchain->impl.swapchain_image_count);
	CHECK(vkGetSwapchainImagesKHR(context.device, swapchain->impl.swapchain, &swapchain->impl.swapchain_image_count, swapchain->impl.images));

	// for (int i = 0; i < swapchain->impl.swapchain_image_count; i++) {
	// 	VkImageViewCreateInfo create_info = {0};
	// 	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	// 	create_info.pNext = NULL;
	// 	create_info.image = swapchain->impl.images[i];
	// 	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	// 	create_info.format = swapchain->impl.format;

	// 	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	// 	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	// 	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	// 	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// 	create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 	create_info.subresourceRange.baseMipLevel = 0;
	// 	create_info.subresourceRange.levelCount = 1;
	// 	create_info.subresourceRange.baseArrayLayer = 0;
	// 	create_info.subresourceRange.layerCount = 1;

	// 	CHECK(vkCreateImageView(context.device, &create_info, NULL, &swapchain->impl.views[i]));
	// }
}

void kinc_g6_swapchain_init(kinc_g6_swapchain_t *swapchain, int window, int width, int height) {
	memset(swapchain, 0, sizeof(*swapchain));
	VkSemaphoreCreateInfo sem_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = NULL, .flags = 0};
	vkCreateSemaphore(context.device, &sem_info, NULL, &swapchain->impl.acquireSem);
	CHECK(kinc_vulkan_create_surface(context.instance, window, &(swapchain->impl.surface)));
	create_swapchain(swapchain, width, height);
}

void kinc_g6_swapchain_resize(kinc_g6_swapchain_t *swapchain, int width, int height) {
	create_swapchain(swapchain, width, height);
}

void kinc_g6_swapchain_destroy(kinc_g6_swapchain_t *swapchain) {
	// for (int i = 0; i < swapchain->impl.swapchain_image_count; i++) {
	// 	vkDestroyImageView(context.device, swapchain->impl.views[i], NULL);
	// }
	if (swapchain->impl.images != NULL) {
		free(swapchain->impl.images);
	}
	// if (swapchain->impl.views != NULL) {
	// 	free(swapchain->impl.views);
	// }
	vkDestroySurfaceKHR(context.instance, swapchain->impl.surface, NULL);
}

kinc_g6_texture_t *kinc_g6_swapchain_next_texture(kinc_g6_swapchain_t *swapchain) {
	uint32_t image_index;
	VkResult r = vkAcquireNextImageKHR(context.device, swapchain->impl.swapchain, UINT64_MAX, swapchain->impl.acquireSem, VK_NULL_HANDLE, &image_index);
	switch (r) {
	case VK_SUBOPTIMAL_KHR:
		break;
	default:
		CHECK(r);
	}
	swapchain->impl.current_index = image_index;
	VkImage image = swapchain->impl.images[image_index];
	kinc_g6_texture_impl_t *impl = &swapchain->impl.swap_texture.impl;
	impl->image = image;
	impl->format = swapchain->impl.format;
	impl->extent.width = swapchain->impl.extent.width;
	impl->extent.height = swapchain->impl.extent.height;
	impl->extent.depth = 0;
	return &swapchain->impl.swap_texture;
}

void kinc_internal_resize(int width, int height) {}
void kinc_internal_change_framebuffer(int window, void *f) {}

static VkSemaphore renderComplete = VK_NULL_HANDLE;

void kinc_g6_submit(kinc_g6_swapchain_t *swapchain, struct kinc_g6_command_buffer **buffers, int count) {
	if (renderComplete == VK_NULL_HANDLE) {
		VkSemaphoreCreateInfo sem = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = 0, .flags = 0};
		vkCreateSemaphore(context.device, &sem, NULL, &renderComplete);
	}
	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = 0;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &swapchain->impl.acquireSem;
	submit_info.pWaitDstStageMask = (VkPipelineStageFlagBits[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.commandBufferCount = count;
	VkCommandBuffer *cbuffers = alloca(sizeof(VkCommandBuffer) * count);
	for (int i = 0; i < count; i++) cbuffers[i] = buffers[i]->impl.buffer;
	submit_info.pCommandBuffers = cbuffers;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &renderComplete;
	vkQueueSubmit(context.queue, 1, &submit_info, VK_NULL_HANDLE);
}

void kinc_g6_present(kinc_g6_swapchain_t *swapchain) {
	VkPresentInfoKHR present_info = {0};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &renderComplete;

	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain->impl.swapchain;
	present_info.pImageIndices = &swapchain->impl.current_index;
	present_info.pResults = NULL;
	VkResult r = vkQueuePresentKHR(context.queue, &present_info);
	switch (r) {
	case VK_SUBOPTIMAL_KHR:
		break;
	default:
		CHECK(r);
	}
}