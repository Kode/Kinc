#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <Kore/Log.h>
#include <Kore/Error.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <vulkan/vulkan.h>

using namespace Kore;

#ifdef SYS_WINDOWS
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        MessageBoxA(NULL, err_msg, err_class, MB_OK);                          \
        exit(1);                                                               \
    } while (0)
#else
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        printf(err_msg);                                                       \
        fflush(stdout);                                                        \
        exit(1);                                                               \
    } while (0)
#endif

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                               \
    {                                                                          \
        fp##entrypoint =                                                       \
            (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint); \
        if (fp##entrypoint == NULL) {                                          \
            ERR_EXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint,    \
                     "vkGetInstanceProcAddr Failure");                         \
        }                                                                      \
    }

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define APP_NAME_STR_LEN 80

VkDevice device;
VkFormat format;
VkFormat depth_format;
VkRenderPass render_pass;
VkCommandBuffer draw_cmd;
VkDescriptorSet desc_set;
VkPhysicalDevice gpu;
VkCommandBuffer setup_cmd; // Command Buffer for initialization commands
VkCommandPool cmd_pool;
VkQueue queue;
bool use_staging_buffer;
VkDescriptorPool desc_pool;
uint32_t swapchainImageCount;

#ifndef NDEBUG
#define VALIDATE
#endif

struct SwapchainBuffers {
	VkImage image;
	VkCommandBuffer cmd;
	VkImageView view;
};

SwapchainBuffers* buffers;

struct DepthBuffer {
	VkImage image;
	VkDeviceMemory mem;
	VkImageView view;
};

DepthBuffer depth;

Texture* vulkanTextures[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
RenderTarget* vulkanRenderTargets[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

void createDescriptorLayout();
void createDescriptorSet(Texture* texture, RenderTarget* renderTarget, VkDescriptorSet& desc_set);

#ifdef SYS_LINUX
extern xcb_connection_t* connection;
extern xcb_screen_t* screen;
extern xcb_window_t window;
extern xcb_intern_atom_reply_t* atom_wm_delete_window;
#endif

namespace {
#ifdef SYS_WINDOWS
	HWND windowHandle;

	HINSTANCE connection;        // hInstance - Windows Instance
	char name[APP_NAME_STR_LEN]; // Name to put on the window/icon
	HWND window;                 // hWnd - window handle
#endif
	VkSurfaceKHR surface;
	bool prepared;
	bool began = false;
	bool onBackBuffer = false;

	VkAllocationCallbacks allocator;

	VkInstance inst;
	VkPhysicalDeviceProperties gpu_props;
	VkQueueFamilyProperties *queue_props;
	uint32_t graphics_queue_node_index;

	uint32_t enabled_extension_count;
#ifdef VALIDATE
	uint32_t enabled_layer_count;
#endif
	char* extension_names[64];
#ifdef VALIDATE
	char* device_validation_layers[64];
#endif

	int width, height;
	VkColorSpaceKHR color_space;

	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	VkSwapchainKHR swapchain;

	VkFramebuffer *framebuffers;

	VkPhysicalDeviceMemoryProperties memory_properties;

	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
	VkDebugReportCallbackEXT msg_callback;

	float depthStencil;
	float depthIncrement;

	bool quit;
	uint32_t current_buffer;
	uint32_t queue_count;

	VkSemaphore presentCompleteSemaphore;

	VkBool32 demo_check_layers(uint32_t check_count, char** check_names, uint32_t layer_count, VkLayerProperties* layers) {
		for (uint32_t i = 0; i < check_count; ++i) {
			VkBool32 found = 0;
			for (uint32_t j = 0; j < layer_count; ++j) {
				if (!strcmp(check_names[i], layers[j].layerName)) {
					found = 1;
					break;
				}
			}
			if (!found) {
				Kore::log(Kore::Error, "Cannot find layer: %s\n", check_names[i]);
				return 0;
			}
		}
		return 1;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData) {
		if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			Kore::log(Kore::Error, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
		}
		else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
			Kore::log(Kore::Warning, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
		}
		return false;
	}

	VKAPI_ATTR void* VKAPI_CALL myrealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		return realloc(pOriginal, size);
	}

	VKAPI_ATTR void* VKAPI_CALL myalloc(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
#ifdef _MSC_VER
		return _aligned_malloc(size, alignment);
#else
		return aligned_alloc(alignment, size);
#endif
	}

	VKAPI_ATTR void VKAPI_CALL myfree(void* pUserData, void* pMemory) {
#ifdef _MSC_VER
		_aligned_free(pMemory);
#else
		free(pMemory);
#endif
	}

	void demo_set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout) {
		VkResult err;

		if (setup_cmd == VK_NULL_HANDLE) {
			VkCommandBufferAllocateInfo cmd = {};
			cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd.pNext = nullptr;
			cmd.commandPool = cmd_pool;
			cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd.commandBufferCount = 1;

			err = vkAllocateCommandBuffers(device, &cmd, &setup_cmd);
			assert(!err);

			VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
			cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			cmd_buf_hinfo.pNext = nullptr;
			cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
			cmd_buf_hinfo.subpass = 0;
			cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
			cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
			cmd_buf_hinfo.queryFlags = 0;
			cmd_buf_hinfo.pipelineStatistics = 0;

			VkCommandBufferBeginInfo cmd_buf_info = {};
			cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmd_buf_info.pNext = nullptr;
			cmd_buf_info.flags = 0;
			cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

			err = vkBeginCommandBuffer(setup_cmd, &cmd_buf_info);
			assert(!err);
		}

		VkImageMemoryBarrier image_memory_barrier = {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.pNext = nullptr;
		image_memory_barrier.srcAccessMask = 0;
		image_memory_barrier.dstAccessMask = 0;
		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange.aspectMask = aspectMask;
		image_memory_barrier.subresourceRange.baseMipLevel = 0;
		image_memory_barrier.subresourceRange.levelCount = 1;
		image_memory_barrier.subresourceRange.baseArrayLayer = 0;
		image_memory_barrier.subresourceRange.layerCount = 1;

		if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			// Make sure anything that was copying from this image has completed
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			// Make sure any Copy or CPU writes to image are flushed
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}

		vkCmdPipelineBarrier(setup_cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	void demo_flush_init_cmd() {
		VkResult err;

		if (setup_cmd == VK_NULL_HANDLE) return;

		err = vkEndCommandBuffer(setup_cmd);
		assert(!err);

		const VkCommandBuffer cmd_bufs[] = { setup_cmd };
		VkFence nullFence = { VK_NULL_HANDLE };
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = NULL;
		submit_info.pWaitDstStageMask = NULL;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = cmd_bufs;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = NULL;

		err = vkQueueSubmit(queue, 1, &submit_info, nullFence);
		assert(!err);

		err = vkQueueWaitIdle(queue);
		assert(!err);

		vkFreeCommandBuffers(device, cmd_pool, 1, cmd_bufs);
		setup_cmd = VK_NULL_HANDLE;
	}

	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int getPower2(int i) {
		for (int power = 0; ; ++power)
			if (pow(power) >= i) return pow(power);
	}
}

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags &
				requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

void Graphics::destroy(int windowId) {

}

#if defined(SYS_WINDOWS)
void Graphics::setup() {
}
#endif

void Graphics::setColorMask(bool red, bool green, bool blue, bool alpha) {

}

#if defined(SYS_WINDOWS)
void Graphics::clearCurrent() {

}
#endif

#if defined(SYS_WINDOWS)
void Graphics::makeCurrent(int contextId) {

}
#endif

void Graphics::init(int windowId, int depthBufferBits, int stencilBufferBits) {
	uint32_t instance_extension_count = 0;
	uint32_t instance_layer_count = 0;
#ifdef VALIDATE
	uint32_t device_validation_layer_count = 0;
#endif
	//demo->enabled_extension_count = 0;
	//demo->enabled_layer_count = 0;

#ifdef VALIDATE
	char* instance_validation_layers[] = {
		//"VK_LAYER_LUNARG_mem_tracker",
		//"VK_LAYER_GOOGLE_unique_objects",
		"VK_LAYER_LUNARG_standard_validation"
	};
#endif

#ifdef VALIDATE
	//device_validation_layers[0] = "VK_LAYER_LUNARG_mem_tracker";
	//device_validation_layers[1] = "VK_LAYER_GOOGLE_unique_objects";
	device_validation_layers[0] = "VK_LAYER_LUNARG_standard_validation";
	device_validation_layer_count = 1;
#endif

	VkBool32 validation_found = 0;
	VkResult err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
	assert(!err);

	if (instance_layer_count > 0) {
		VkLayerProperties* instance_layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
		assert(!err);

#ifdef VALIDATE
			validation_found = demo_check_layers(
				ARRAY_SIZE(instance_validation_layers),
				instance_validation_layers, instance_layer_count,
				instance_layers);
			enabled_layer_count = ARRAY_SIZE(instance_validation_layers);
#endif
		free(instance_layers);
	}

#ifdef VALIDATE
	if (!validation_found) {
		ERR_EXIT("vkEnumerateInstanceLayerProperties failed to find"
			"required validation layer.\n\n"
			"Please look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateInstance Failure");
	}
#endif

	/* Look for instance extensions */
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
	memset(extension_names, 0, sizeof(extension_names));

	err = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
	assert(!err);

	if (instance_extension_count > 0) {
		VkExtensionProperties* instance_extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
		err = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions);
		assert(!err);
		for (uint32_t i = 0; i < instance_extension_count; i++) {
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				surfaceExtFound = 1;
				extension_names[enabled_extension_count++] =
					VK_KHR_SURFACE_EXTENSION_NAME;
			}
#ifdef SYS_WINDOWS
			if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				platformSurfaceExtFound = 1;
				extension_names[enabled_extension_count++] =
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}
#else
			if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                extension_names[enabled_extension_count++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
            }
#endif

			if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName)) {
#ifdef VALIDATE
					extension_names[enabled_extension_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
#endif
			}
			assert(enabled_extension_count < 64);
		}

		free(instance_extensions);
	}

	if (!surfaceExtFound) {
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
			"the " VK_KHR_SURFACE_EXTENSION_NAME
			" extension.\n\nDo you have a compatible "
			"Vulkan installable client driver (ICD) installed?\nPlease "
			"look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateInstance Failure");
	}
	if (!platformSurfaceExtFound) {
#ifdef SYS_WINDOWS
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
			"the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			" extension.\n\nDo you have a compatible "
			"Vulkan installable client driver (ICD) installed?\nPlease "
			"look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateInstance Failure");
#else
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
                 "the " VK_KHR_XCB_SURFACE_EXTENSION_NAME
                 " extension.\n\nDo you have a compatible "
                 "Vulkan installable client driver (ICD) installed?\nPlease "
                 "look at the Getting Started guide for additional "
                 "information.\n",
                 "vkCreateInstance Failure");
#endif
	}
	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = nullptr;
	app.pApplicationName = System::name();
	app.applicationVersion = 0;
	app.pEngineName = "Kore";
	app.engineVersion = 0;
	app.apiVersion = VK_MAKE_VERSION(1, 0, 3); //VK_API_VERSION;

	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = nullptr;
	info.pApplicationInfo = &app;
#ifdef VALIDATE
	info.enabledLayerCount = enabled_layer_count;
	info.ppEnabledLayerNames = (const char *const *)instance_validation_layers;
#else
	info.enabledLayerCount = 0;
	info.ppEnabledLayerNames = nullptr;
#endif
	info.enabledExtensionCount = enabled_extension_count;
	info.ppEnabledExtensionNames = (const char *const *)extension_names;

	uint32_t gpu_count;

	allocator.pfnAllocation = myalloc;
	allocator.pfnFree = myfree;
	allocator.pfnReallocation = myrealloc;

	err = vkCreateInstance(&info, &allocator, &inst);
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
		ERR_EXIT("Cannot find a compatible Vulkan installable client driver "
			"(ICD).\n\nPlease look at the Getting Started guide for "
			"additional information.\n",
			"vkCreateInstance Failure");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
		ERR_EXIT("Cannot find a specified extension library"
			".\nMake sure your layers path is set appropriately\n",
			"vkCreateInstance Failure");
	}
	else if (err) {
		ERR_EXIT("vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
			"installable client driver (ICD) installed?\nPlease look at "
			"the Getting Started guide for additional information.\n",
			"vkCreateInstance Failure");
	}

	/* Make initial call to query gpu_count, then second call for gpu info*/
	err = vkEnumeratePhysicalDevices(inst, &gpu_count, NULL);
	assert(!err && gpu_count > 0);

	if (gpu_count > 0) {
		VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(inst, &gpu_count, physical_devices);
		assert(!err);
		/* For tri demo we just grab the first physical device */
		gpu = physical_devices[0];
		free(physical_devices);
	}
	else {
		ERR_EXIT("vkEnumeratePhysicalDevices reported zero accessible devices."
			"\n\nDo you have a compatible Vulkan installable client"
			" driver (ICD) installed?\nPlease look at the Getting Started"
			" guide for additional information.\n",
			"vkEnumeratePhysicalDevices Failure");
	}

	/* Look for validation layers */
	validation_found = 0;
#ifdef VALIDATE
	enabled_layer_count = 0;
#endif
	uint32_t device_layer_count = 0;
	err = vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, NULL);
	assert(!err);

	if (device_layer_count > 0) {
		VkLayerProperties* device_layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * device_layer_count);
		err = vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layers);
		assert(!err);

#ifdef VALIDATE
			validation_found = demo_check_layers(device_validation_layer_count,
				device_validation_layers,
				device_layer_count,
				device_layers);
			enabled_layer_count = device_validation_layer_count;
#endif

		free(device_layers);
	}

#ifdef VALIDATE
	if (!validation_found) {
		ERR_EXIT("vkEnumerateDeviceLayerProperties failed to find "
			"a required validation layer.\n\n"
			"Please look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateDevice Failure");
	}
#endif

	/* Loog for device extensions */
	uint32_t device_extension_count = 0;
	VkBool32 swapchainExtFound = 0;
	enabled_extension_count = 0;
	memset(extension_names, 0, sizeof(extension_names));

	err = vkEnumerateDeviceExtensionProperties(gpu, NULL,
		&device_extension_count, NULL);
	assert(!err);

	if (device_extension_count > 0) {
		VkExtensionProperties* device_extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * device_extension_count);
		err = vkEnumerateDeviceExtensionProperties(gpu, NULL, &device_extension_count, device_extensions);
		assert(!err);

		for (uint32_t i = 0; i < device_extension_count; i++) {
			if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				device_extensions[i].extensionName)) {
				swapchainExtFound = 1;
				extension_names[enabled_extension_count++] =
					VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}
			assert(enabled_extension_count < 64);
		}

		free(device_extensions);
	}

	if (!swapchainExtFound) {
		ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find "
			"the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
			" extension.\n\nDo you have a compatible "
			"Vulkan installable client driver (ICD) installed?\nPlease "
			"look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateInstance Failure");
	}

#ifdef VALIDATE
		CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugReportCallbackEXT");
		if (!CreateDebugReportCallback) {
			ERR_EXIT("GetProcAddr: Unable to find vkCreateDebugReportCallbackEXT\n", "vkGetProcAddr Failure");
		}
		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.flags =
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		dbgCreateInfo.pfnCallback = dbgFunc;
		dbgCreateInfo.pUserData = NULL;
		dbgCreateInfo.pNext = NULL;
		err = CreateDebugReportCallback(inst, &dbgCreateInfo, NULL, &msg_callback);
		switch (err) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			ERR_EXIT("CreateDebugReportCallback: out of host memory\n", "CreateDebugReportCallback Failure");
			break;
		default:
			ERR_EXIT("CreateDebugReportCallback: unknown failure\n", "CreateDebugReportCallback Failure");
			break;
		}
#endif

	// Having these GIPA queries of device extension entry points both
	// BEFORE and AFTER vkCreateDevice is a good test for the loader
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(inst, CreateSwapchainKHR);
	GET_INSTANCE_PROC_ADDR(inst, DestroySwapchainKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetSwapchainImagesKHR);
	GET_INSTANCE_PROC_ADDR(inst, AcquireNextImageKHR);
	GET_INSTANCE_PROC_ADDR(inst, QueuePresentKHR);

	vkGetPhysicalDeviceProperties(gpu, &gpu_props);

	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);

	queue_props = (VkQueueFamilyProperties*)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);
	assert(queue_count >= 1);

	width = System::windowWidth(windowId);
	height = System::windowHeight(windowId);
	depthStencil = 1.0;
	depthIncrement = -0.01f;

#ifdef SYS_WINDOWS
	windowHandle = (HWND)System::windowHandle(windowId);
	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle); // Slightly Higher Priority
	SetFocus(windowHandle); // Sets Keyboard Focus To The Window
#endif

	{
		VkResult err;
		uint32_t i;
#ifdef SYS_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext = NULL;
		createInfo.flags = 0;
		createInfo.hinstance = connection;
		createInfo.hwnd = windowHandle;
		err = vkCreateWin32SurfaceKHR(inst, &createInfo, NULL, &surface);
		assert(!err);
#else
        VkXcbSurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.connection = connection;
        createInfo.window = window;
        err = vkCreateXcbSurfaceKHR(inst, &createInfo, nullptr, &surface);
        assert(!err);
#endif
		// Iterate over each queue to learn whether it supports presenting:
		VkBool32 *supportsPresent =
			(VkBool32 *)malloc(queue_count * sizeof(VkBool32));
		for (i = 0; i < queue_count; i++) {
			fpGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supportsPresent[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (i = 0; i < queue_count; i++) {
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
		if (graphicsQueueNodeIndex == UINT32_MAX ||
			presentQueueNodeIndex == UINT32_MAX) {
			ERR_EXIT("Could not find a graphics and a present queue\n",
				"Swapchain Initialization Failure");
		}

		// TODO: Add support for separate queues, including presentation,
		//       synchronization, and appropriate tracking for QueueSubmit.
		// NOTE: While it is possible for an application to use a separate graphics
		//       and a present queues, this demo program assumes it is only using
		//       one:
		if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
			ERR_EXIT("Could not find a common graphics and a present queue\n",
				"Swapchain Initialization Failure");
		}

		graphics_queue_node_index = graphicsQueueNodeIndex;

		{
			float queue_priorities[1] = { 0.0 };
			VkDeviceQueueCreateInfo queue = {};
			queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue.pNext = nullptr;
			queue.queueFamilyIndex = graphics_queue_node_index;
			queue.queueCount = 1;
			queue.pQueuePriorities = queue_priorities;

			VkDeviceCreateInfo deviceinfo = {};
			deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceinfo.pNext = nullptr;
			deviceinfo.queueCreateInfoCount = 1;
			deviceinfo.pQueueCreateInfos = &queue;
#ifdef VALIDATE
			deviceinfo.enabledLayerCount = enabled_layer_count;
			deviceinfo.ppEnabledLayerNames = (const char *const *)device_validation_layers;
#else
			deviceinfo.enabledLayerCount = 0;
			deviceinfo.ppEnabledLayerNames = nullptr;
#endif
			deviceinfo.enabledExtensionCount = enabled_extension_count;
			deviceinfo.ppEnabledExtensionNames = (const char *const *)extension_names;

			err = vkCreateDevice(gpu, &deviceinfo, nullptr, &device);
			assert(!err);
		}

		vkGetDeviceQueue(device, graphics_queue_node_index, 0, &queue);

		// Get the list of VkFormat's that are supported:
		uint32_t formatCount;
		err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
		assert(!err);
		VkSurfaceFormatKHR* surfFormats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
		err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, surfFormats);
		assert(!err);
		// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
		// the surface has no preferred format.  Otherwise, at least one
		// supported format will be returned.
		if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
			format = VK_FORMAT_B8G8R8A8_UNORM;
		}
		else {
			assert(formatCount >= 1);
			format = surfFormats[0].format;
		}
		color_space = surfFormats[0].colorSpace;

		// Get Memory information and properties
		vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
	}

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = graphics_queue_node_index;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	err = vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmd_pool);
	assert(!err);

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	err = vkAllocateCommandBuffers(device, &cmd, &draw_cmd);
	assert(!err);

	VkSwapchainKHR oldSwapchain = swapchain;

	// Check the surface capabilities and formats
	VkSurfaceCapabilitiesKHR surfCapabilities = {};
	err = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfCapabilities);
	assert(!err);

	uint32_t presentModeCount;
	err = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, NULL);
	assert(!err);
	VkPresentModeKHR* presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	assert(presentModes);
	err = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes);
	assert(!err);

	VkExtent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = width;
		swapchainExtent.height = height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		width = surfCapabilities.currentExtent.width;
		height = surfCapabilities.currentExtent.height;
	}

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) &&
		(desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform = {};
	if (surfCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchain_info = {};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.pNext = NULL;
	swapchain_info.surface = surface;
	swapchain_info.minImageCount = desiredNumberOfSwapchainImages;
	swapchain_info.imageFormat = format;
	swapchain_info.imageColorSpace = color_space;
	swapchain_info.imageExtent.width = swapchainExtent.width;
	swapchain_info.imageExtent.height = swapchainExtent.height;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.preTransform = preTransform;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.queueFamilyIndexCount = 0;
	swapchain_info.pQueueFamilyIndices = NULL;
	swapchain_info.presentMode = swapchainPresentMode;
	swapchain_info.oldSwapchain = oldSwapchain;
	swapchain_info.clipped = true;

	uint32_t i;

	err = fpCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain);
	assert(!err);

	// If we just re-created an existing swapchain, we should destroy the old
	// swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated
	// presentable images once the platform is done with them.
	if (oldSwapchain != VK_NULL_HANDLE) {
		fpDestroySwapchainKHR(device, oldSwapchain, NULL);
	}

	err = fpGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
	assert(!err);

	VkImage* swapchainImages = (VkImage*)malloc(swapchainImageCount * sizeof(VkImage));
	assert(swapchainImages);
	err = fpGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);
	assert(!err);

	buffers = (SwapchainBuffers *)malloc(sizeof(SwapchainBuffers) * swapchainImageCount);
	assert(buffers);

	for (i = 0; i < swapchainImageCount; i++) {
		VkImageViewCreateInfo color_attachment_view = {};
		color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_attachment_view.pNext = NULL;
		color_attachment_view.format = format;
		color_attachment_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_attachment_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_attachment_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_attachment_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_attachment_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_attachment_view.subresourceRange.baseMipLevel = 0;
		color_attachment_view.subresourceRange.levelCount = 1;
		color_attachment_view.subresourceRange.baseArrayLayer = 0;
		color_attachment_view.subresourceRange.layerCount = 1;
		color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_attachment_view.flags = 0;

		buffers[i].image = swapchainImages[i];

		// Render loop will expect image to have been used before and in
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		// layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
		// to that state
		demo_set_image_layout(buffers[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		color_attachment_view.image = buffers[i].image;

		err = vkCreateImageView(device, &color_attachment_view, NULL, &buffers[i].view);
		assert(!err);
	}

	current_buffer = 0;

	if (NULL != presentModes) {
		free(presentModes);
	}

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.pNext = NULL;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = depth_format;
	image.extent.width = width;
	image.extent.height = height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo view = {};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.format = depth_format;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VkMemoryRequirements mem_reqs = {};
	bool pass;

	::depth_format = depth_format;

	/* create image */
	err = vkCreateImage(device, &image, NULL, &depth.image);
	assert(!err);

	/* get memory requirements for this object */
	vkGetImageMemoryRequirements(device, depth.image, &mem_reqs);

	/* select memory size and type */
	mem_alloc.allocationSize = mem_reqs.size;
	pass = memory_type_from_properties(mem_reqs.memoryTypeBits, 0, /* No requirements */ &mem_alloc.memoryTypeIndex);
	assert(pass);

	/* allocate memory */
	err = vkAllocateMemory(device, &mem_alloc, NULL, &depth.mem);
	assert(!err);

	/* bind memory */
	err = vkBindImageMemory(device, depth.image, depth.mem, 0);
	assert(!err);

	demo_set_image_layout(depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	/* create image view */
	view.image = depth.image;
	err = vkCreateImageView(device, &view, NULL, &depth.view);
	assert(!err);

	VkAttachmentDescription attachments[2];
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[1].format = depth_format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = &depth_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = nullptr;
	rp_info.attachmentCount = 2;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = nullptr;

	err = vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
	assert(!err);

	{
		VkImageView attachments[2];
		attachments[1] = depth.view;

		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = NULL;
		fb_info.renderPass = render_pass;
		fb_info.attachmentCount = 2;
		fb_info.pAttachments = attachments;
		fb_info.width = width;
		fb_info.height = height;
		fb_info.layers = 1;

		uint32_t i;

		framebuffers = (VkFramebuffer*)malloc(swapchainImageCount * sizeof(VkFramebuffer));
		assert(framebuffers);

		for (i = 0; i < swapchainImageCount; i++) {
			attachments[0] = buffers[i].view;
			err = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);
			assert(!err);
		}
	}

	createDescriptorLayout();
	createDescriptorSet(nullptr, nullptr, desc_set);

	begin();
}

unsigned Graphics::refreshRate() {
	return 60;
}

bool Graphics::vsynced() {
	return false;
}

//void* Graphics::getControl() {
//	return nullptr;
//}

void Graphics::setBool(ConstantLocation location, bool value) {
	if (location.vertexOffset >= 0) {
		int* data = (int*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		data[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		int* data = (int*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		data[0] = value;
	}
}

void Graphics::setInt(ConstantLocation location, int value) {
	if (location.vertexOffset >= 0) {
		int* data = (int*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		data[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		int* data = (int*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		data[0] = value;
	}
}

void Graphics::setFloat(ConstantLocation location, float value) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		data[0] = value;
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		data[0] = value;
	}
}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		data[0] = value1;
		data[1] = value2;
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		data[0] = value1;
		data[1] = value2;
	}
}

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		data[0] = value1;
		data[1] = value2;
		data[2] = value3;
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		data[0] = value1;
		data[1] = value2;
		data[2] = value3;
	}
}

void Graphics::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		data[0] = value1;
		data[1] = value2;
		data[2] = value3;
		data[3] = value4;
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		data[0] = value1;
		data[1] = value2;
		data[2] = value3;
		data[3] = value4;
	}
}

void Graphics::setFloats(ConstantLocation location, float* values, int count) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		for (int i = 0; i < count; ++i) {
			data[i] = values[i];
		}
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		for (int i = 0; i < count; ++i) {
			data[i] = values[i];
		}
	}
}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		for (int i = 0; i < 16; ++i) {
			data[i] = value.data[i];
		}
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		for (int i = 0; i < 16; ++i) {
			data[i] = value.data[i];
		}
	}
}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {
	if (location.vertexOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataVertex)[location.vertexOffset];
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				data[y * 4 + x] = value.data[y * 3 + x];
			}
		}
	}
	if (location.fragmentOffset >= 0) {
		float* data = (float*)&((u8*)ProgramImpl::current->uniformDataFragment)[location.fragmentOffset];
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				data[y * 4 + x] = value.data[y * 3 + x];
			}
		}
	}
}

void Graphics::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {
	vkCmdDrawIndexed(draw_cmd, count, 1, start, 0, 0);
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {
	//drawIndexedVerticesInstanced(instanceCount, 0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {

}

void Graphics::swapBuffers(int contextId) {

}

void Graphics::begin(int contextId) {
	if (began) return;

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {};
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	VkResult err = vkCreateSemaphore(device, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);
	assert(!err);

	// Get the index of the next available swapchain image:
	err = fpAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)0, // TODO: Show use of fence
		&current_buffer);
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		// demo->swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		//demo_resize(demo);
		//demo_draw(demo);
		error("VK_ERROR_OUT_OF_DATE_KHR");
		vkDestroySemaphore(device, presentCompleteSemaphore, NULL);
		return;
	}
	else if (err == VK_SUBOPTIMAL_KHR) {
		// demo->swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
	}
	else {
		assert(!err);
	}

	// Assume the command buffer has been run on current_buffer before so
	// we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
	demo_set_image_layout(buffers[current_buffer].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	demo_flush_init_cmd();

	VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
	cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cmd_buf_hinfo.pNext = NULL;
	cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
	cmd_buf_hinfo.subpass = 0;
	cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
	cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
	cmd_buf_hinfo.queryFlags = 0;
	cmd_buf_hinfo.pipelineStatistics = 0;

	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = depthStencil;
	clear_values[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin = {};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = render_pass;
	rp_begin.framebuffer = framebuffers[current_buffer];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = width;
	rp_begin.renderArea.extent.height = height;
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;

	err = vkBeginCommandBuffer(draw_cmd, &cmd_buf_info);
	assert(!err);
	vkCmdBeginRenderPass(draw_cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(draw_cmd, 0, 1, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(draw_cmd, 0, 1, &scissor);

	began = true;
	onBackBuffer = true;
}

void Graphics::viewport(int x, int y, int width, int height) {

}

void Graphics::scissor(int x, int y, int width, int height) {

}

void Graphics::disableScissor() {

}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask, int writeMask) {

}

void Graphics::end(int windowId) {
	vkCmdEndRenderPass(draw_cmd);

	VkImageMemoryBarrier prePresentBarrier = {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	prePresentBarrier.image = buffers[current_buffer].image;
	VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
	vkCmdPipelineBarrier(draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);

	VkResult err = vkEndCommandBuffer(draw_cmd);
	assert(!err);

	VkFence nullFence = VK_NULL_HANDLE;
	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &presentCompleteSemaphore;
	submit_info.pWaitDstStageMask = &pipe_stage_flags;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &draw_cmd;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;

	err = vkQueueSubmit(queue, 1, &submit_info, nullFence);
	assert(!err);

	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &swapchain;
	present.pImageIndices = &current_buffer;

	// TBD/TODO: SHOULD THE "present" PARAMETER BE "const" IN THE HEADER?
	err = fpQueuePresentKHR(queue, &present);
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		// demo->swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		//demo_resize(demo);
		error("VK_ERROR_OUT_OF_DATE_KHR");
	}
	else if (err == VK_SUBOPTIMAL_KHR) {
		// demo->swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
	}
	else {
		assert(!err);
	}

	err = vkQueueWaitIdle(queue);
	assert(err == VK_SUCCESS);

	vkDestroySemaphore(device, presentCompleteSemaphore, NULL);

	if (depthStencil > 0.99f) depthIncrement = -0.001f;
	if (depthStencil < 0.8f) depthIncrement = 0.001f;

	depthStencil += depthIncrement;

#ifndef SYS_WINDOWS
	vkDeviceWaitIdle(device);
#endif

	began = false;
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {
	/*VkClearColorValue clearColor = {};
	clearColor.float32[0] = 1.0f;
	clearColor.float32[1] = 0.0f;
	clearColor.float32[2] = 0.0f;
	clearColor.float32[3] = 1.0f;
	VkImageSubresourceRange range = {};
	range.levelCount = 1;
	range.layerCount = 1;
	range.aspectMask = 0;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	vkCmdClearColorImage(draw_cmd, buffers[current_buffer].image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);*/
}

void Graphics::setRenderState(RenderState state, bool on) {

}

void Graphics::setRenderState(RenderState state, int v) {

}

void Graphics::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {
	vertexBuffers[0]->_set();
}

void Graphics::setIndexBuffer(IndexBuffer& indexBuffer) {
	indexBuffer._set();
}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {
	vulkanTextures[unit.binding - 2] = texture;
	vulkanRenderTargets[unit.binding - 2] = nullptr;
	if (ProgramImpl::current != nullptr) vkCmdBindDescriptorSets(draw_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ProgramImpl::current->pipeline_layout, 0, 1, &texture->desc_set, 0, NULL);
}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {

}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {

}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {

}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {

}

void setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

namespace {
	RenderTarget* currentRenderTarget = nullptr;

	void endPass() {
		vkCmdEndRenderPass(draw_cmd);

		if (currentRenderTarget == nullptr) {
			/*VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = NULL;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			barrier.image = buffers[current_buffer].image;

			vkCmdPipelineBarrier(draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);*/
		}
		else {
			setImageLayout(currentRenderTarget->sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			setImageLayout(currentRenderTarget->destImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkImageBlit imgBlit;

			imgBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgBlit.srcSubresource.mipLevel = 0;
			imgBlit.srcSubresource.baseArrayLayer = 0;
			imgBlit.srcSubresource.layerCount = 1;

			imgBlit.srcOffsets[0] = { 0, 0, 0 };
			imgBlit.srcOffsets[1].x = currentRenderTarget->width;
			imgBlit.srcOffsets[1].y = currentRenderTarget->height;
			imgBlit.srcOffsets[1].z = 1;

			imgBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgBlit.dstSubresource.mipLevel = 0;
			imgBlit.dstSubresource.baseArrayLayer = 0;
			imgBlit.dstSubresource.layerCount = 1;

			imgBlit.dstOffsets[0] = { 0, 0, 0 };
			imgBlit.dstOffsets[1].x = currentRenderTarget->width;
			imgBlit.dstOffsets[1].y = currentRenderTarget->height;
			imgBlit.dstOffsets[1].z = 1;

			vkCmdBlitImage(draw_cmd, currentRenderTarget->sourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, currentRenderTarget->destImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgBlit, VK_FILTER_LINEAR);

			setImageLayout(currentRenderTarget->sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			setImageLayout(currentRenderTarget->destImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		/*VkResult err = vkEndCommandBuffer(draw_cmd);
		assert(!err);

		VkFence nullFence = VK_NULL_HANDLE;
		VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &presentCompleteSemaphore;
		submit_info.pWaitDstStageMask = &pipe_stage_flags;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &draw_cmd;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = NULL;

		err = vkQueueSubmit(queue, 1, &submit_info, nullFence);
		assert(!err);

		VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
		cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		cmd_buf_hinfo.pNext = NULL;
		cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
		cmd_buf_hinfo.subpass = 0;
		cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
		cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
		cmd_buf_hinfo.queryFlags = 0;
		cmd_buf_hinfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo cmd_buf_info = {};
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = NULL;
		cmd_buf_info.flags = 0;
		cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

		VkClearValue clear_values[2];
		memset(clear_values, 0, sizeof(VkClearValue) * 2);
		clear_values[0].color.float32[0] = 0.0f;
		clear_values[0].color.float32[1] = 0.0f;
		clear_values[0].color.float32[2] = 0.0f;
		clear_values[0].color.float32[3] = 1.0f;
		clear_values[1].depthStencil.depth = depthStencil;
		clear_values[1].depthStencil.stencil = 0;

		VkRenderPassBeginInfo rp_begin = {};
		rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin.pNext = NULL;
		rp_begin.renderPass = render_pass;
		rp_begin.framebuffer = framebuffers[current_buffer];
		rp_begin.renderArea.offset.x = 0;
		rp_begin.renderArea.offset.y = 0;
		rp_begin.renderArea.extent.width = width;
		rp_begin.renderArea.extent.height = height;
		rp_begin.clearValueCount = 2;
		rp_begin.pClearValues = clear_values;

		err = vkBeginCommandBuffer(draw_cmd, &cmd_buf_info);
		assert(!err);*/
	}
}

void Graphics::setRenderTarget(RenderTarget* texture, int num, int additionalTargets) {
	endPass();

	currentRenderTarget = texture;
	onBackBuffer = false;

	VkClearValue clear_values[1];
	memset(clear_values, 0, sizeof(VkClearValue));
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;

	VkRenderPassBeginInfo rp_begin = {};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = nullptr;
	rp_begin.renderPass = texture->renderPass;
	rp_begin.framebuffer = texture->framebuffer;
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = texture->width;
	rp_begin.renderArea.extent.height = texture->height;
	rp_begin.clearValueCount = 1;
	rp_begin.pClearValues = clear_values;

	vkCmdBeginRenderPass(draw_cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.width = (float)texture->width;
	viewport.height = (float)texture->height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(draw_cmd, 0, 1, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = texture->width;
	scissor.extent.height = texture->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(draw_cmd, 0, 1, &scissor);
}

void Graphics::restoreRenderTarget() {
	if (onBackBuffer) return;

	endPass();

	currentRenderTarget = nullptr;
	onBackBuffer = true;

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = depthStencil;
	clear_values[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin = {};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = nullptr;
	rp_begin.renderPass = render_pass;
	rp_begin.framebuffer = framebuffers[current_buffer];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = width;
	rp_begin.renderArea.extent.height = height;
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;

	vkCmdBeginRenderPass(draw_cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(draw_cmd, 0, 1, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(draw_cmd, 0, 1, &scissor);
}

bool Graphics::renderTargetsInvertedY() {
	return true;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}

void Graphics::flush() {

}
