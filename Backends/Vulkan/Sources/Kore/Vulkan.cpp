#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Application.h>
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <Kore/Log.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <Windows.h>

using namespace Kore;

#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        MessageBoxA(NULL, err_msg, err_class, MB_OK);                          \
        exit(1);                                                               \
    } while (0)

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

namespace {
	struct SwapchainBuffers {
		VkImage image;
		VkCommandBuffer cmd;
		VkImageView view;
	};

	HINSTANCE connection;        // hInstance - Windows Instance
	char name[APP_NAME_STR_LEN]; // Name to put on the window/icon
	HWND window;                 // hWnd - window handle

	VkSurfaceKHR surface;
	bool prepared;
	bool use_staging_buffer;

	VkAllocationCallbacks allocator;

	VkInstance inst;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue queue;
	VkPhysicalDeviceProperties gpu_props;
	VkQueueFamilyProperties *queue_props;
	uint32_t graphics_queue_node_index;

	uint32_t enabled_extension_count;
	uint32_t enabled_layer_count;
	char *extension_names[64];
	char *device_validation_layers[64];

	int width, height;
	VkFormat format;
	VkColorSpaceKHR color_space;

	PFN_vkGetPhysicalDeviceSurfaceSupportKHR
		fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
		fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR
		fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
		fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	uint32_t swapchainImageCount;
	VkSwapchainKHR swapchain;
	SwapchainBuffers *buffers;

	VkCommandPool cmd_pool;

	struct {
		VkFormat format;

		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} depth;

	//struct texture_object textures[DEMO_TEXTURE_COUNT];

	struct {
		VkBuffer buf;
		VkDeviceMemory mem;

		VkPipelineVertexInputStateCreateInfo vi;
		VkVertexInputBindingDescription vi_bindings[1];
		VkVertexInputAttributeDescription vi_attrs[2];
	} vertices;

	VkCommandBuffer setup_cmd; // Command Buffer for initialization commands
	VkCommandBuffer draw_cmd;  // Command Buffer for drawing commands
	VkPipelineLayout pipeline_layout;
	VkDescriptorSetLayout desc_layout;
	VkPipelineCache pipelineCache;
	VkRenderPass render_pass;
	VkPipeline pipeline;

	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	VkDescriptorPool desc_pool;
	VkDescriptorSet desc_set;

	VkFramebuffer *framebuffers;

	VkPhysicalDeviceMemoryProperties memory_properties;

	bool validate;
	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
	VkDebugReportCallbackEXT msg_callback;

	float depthStencil;
	float depthIncrement;

	bool quit;
	uint32_t current_buffer;
	uint32_t queue_count;

	VkBool32 demo_check_layers(uint32_t check_count, char **check_names,
		uint32_t layer_count,
		VkLayerProperties *layers) {
		for (uint32_t i = 0; i < check_count; i++) {
			VkBool32 found = 0;
			for (uint32_t j = 0; j < layer_count; j++) {
				if (!strcmp(check_names[i], layers[j].layerName)) {
					found = 1;
					break;
				}
			}
			if (!found) {
				fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
				return 0;
			}
		}
		return 1;
	}


	VKAPI_ATTR VkBool32 VKAPI_CALL
		dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
			uint64_t srcObject, size_t location, int32_t msgCode,
			const char *pLayerPrefix, const char *pMsg, void *pUserData) {
		char *message = (char *)malloc(strlen(pMsg) + 100);

		assert(message);

		if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			sprintf(message, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode,
				pMsg);
		}
		else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
			sprintf(message, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode,
				pMsg);
		}
		else {
			return false;
		}

		MessageBoxA(NULL, message, "Alert", MB_OK);

		free(message);

		/*
		* false indicates that layer should not bail-out of an
		* API call that had validation failures. This may mean that the
		* app dies inside the driver due to invalid parameter(s).
		* That's what would happen without validation layers, so we'll
		* keep that behavior here.
		*/
		return false;
	}


	VKAPI_ATTR void *VKAPI_CALL myrealloc(void *pUserData, void *pOriginal,
		size_t size, size_t alignment,
		VkSystemAllocationScope allocationScope) {
		return realloc(pOriginal, size);
	}

	VKAPI_ATTR void *VKAPI_CALL myalloc(void *pUserData, size_t size,
		size_t alignment,
		VkSystemAllocationScope allocationScope) {
#ifdef _MSC_VER
		return _aligned_malloc(size, alignment);
#else
		return aligned_alloc(alignment, size);
#endif
	}

	VKAPI_ATTR void VKAPI_CALL myfree(void *pUserData, void *pMemory) {
#ifdef _MSC_VER
		_aligned_free(pMemory);
#else
		free(pMemory);
#endif
	}
}

void Graphics::destroy() {

}

void Graphics::init() {
	uint32_t instance_extension_count = 0;
	uint32_t instance_layer_count = 0;
	uint32_t device_validation_layer_count = 0;
	//demo->enabled_extension_count = 0;
	//demo->enabled_layer_count = 0;

	char *instance_validation_layers[] = {
		"VK_LAYER_LUNARG_mem_tracker",
		"VK_LAYER_GOOGLE_unique_objects",
	};

	device_validation_layers[0] = "VK_LAYER_LUNARG_mem_tracker";
	device_validation_layers[1] = "VK_LAYER_GOOGLE_unique_objects";
	device_validation_layer_count = 2;

	VkBool32 validation_found = 0;
	VkResult err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
	assert(!err);
	
	if (instance_layer_count > 0) {
		VkLayerProperties* instance_layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
		assert(!err);

		if (validate) {
			validation_found = demo_check_layers(
				ARRAY_SIZE(instance_validation_layers),
				instance_validation_layers, instance_layer_count,
				instance_layers);
			enabled_layer_count = ARRAY_SIZE(instance_validation_layers);
		}

		free(instance_layers);
	}

	if (validate && !validation_found) {
		ERR_EXIT("vkEnumerateInstanceLayerProperties failed to find"
			"required validation layer.\n\n"
			"Please look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateInstance Failure");
	}

	/* Look for instance extensions */
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
	memset(extension_names, 0, sizeof(extension_names));

	err = vkEnumerateInstanceExtensionProperties(
		NULL, &instance_extension_count, NULL);
	assert(!err);

	if (instance_extension_count > 0) {
		VkExtensionProperties* instance_extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
		err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
		assert(!err);
		for (uint32_t i = 0; i < instance_extension_count; i++) {
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				surfaceExtFound = 1;
				extension_names[enabled_extension_count++] =
					VK_KHR_SURFACE_EXTENSION_NAME;
			}

			if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				platformSurfaceExtFound = 1;
				extension_names[enabled_extension_count++] =
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}

			if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				if (validate) {
					extension_names[enabled_extension_count++] =
						VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
				}
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
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
			"the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			" extension.\n\nDo you have a compatible "
			"Vulkan installable client driver (ICD) installed?\nPlease "
			"look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateInstance Failure");

	}
	VkApplicationInfo app;
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL,
	app.pApplicationName = Application::the()->name();
	app.applicationVersion = 0;
	app.pEngineName = "Kore";
	app.engineVersion = 0;
	app.apiVersion = VK_API_VERSION;
	
	VkInstanceCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.pApplicationInfo = &app;
	info.enabledLayerCount = enabled_layer_count;
	info.ppEnabledLayerNames = (const char *const *)instance_validation_layers;
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
	enabled_layer_count = 0;
	uint32_t device_layer_count = 0;
	err = vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, NULL);
	assert(!err);

	if (device_layer_count > 0) {
		VkLayerProperties* device_layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * device_layer_count);
		err = vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layers);
		assert(!err);

		if (validate) {
			validation_found = demo_check_layers(device_validation_layer_count,
				device_validation_layers,
				device_layer_count,
				device_layers);
			enabled_layer_count = device_validation_layer_count;
		}

		free(device_layers);
	}

	if (validate && !validation_found) {
		ERR_EXIT("vkEnumerateDeviceLayerProperties failed to find "
			"a required validation layer.\n\n"
			"Please look at the Getting Started guide for additional "
			"information.\n",
			"vkCreateDevice Failure");
	}

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

	if (validate) {
		CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugReportCallbackEXT");
		if (!CreateDebugReportCallback) {
			ERR_EXIT("GetProcAddr: Unable to find vkCreateDebugReportCallbackEXT\n", "vkGetProcAddr Failure");
		}
		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
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
	}

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
}

unsigned Graphics::refreshRate() {
	return 60;
}

bool Graphics::vsynced() {
	return true;
}

void* Graphics::getControl() {
	return nullptr;
}

void Graphics::setBool(ConstantLocation location, bool value) {

}

void Graphics::setInt(ConstantLocation location, int value) {

}

void Graphics::setFloat(ConstantLocation location, float value) {

}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {

}

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {

}

void Graphics::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {

}

void Graphics::setFloats(ConstantLocation location, float* values, int count) {

}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {

}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {

}

void Graphics::drawIndexedVertices() {
	//drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {

}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {
	//drawIndexedVerticesInstanced(instanceCount, 0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {

}

void Graphics::swapBuffers() {

}

void Graphics::begin() {

}

void Graphics::viewport(int x, int y, int width, int height) {

}

void Graphics::scissor(int x, int y, int width, int height) {

}

void Graphics::disableScissor() {

}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask, int writeMask) {

}

void Graphics::end() {

}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {

}

void Graphics::setRenderState(RenderState state, bool on) {

}

void Graphics::setRenderState(RenderState state, int v) {
	
}

void Graphics::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {

}

void Graphics::setIndexBuffer(IndexBuffer& indexBuffer) {

}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {

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

void Graphics::setRenderTarget(RenderTarget* texture, int num) {

}

void Graphics::restoreRenderTarget() {

}

bool Graphics::renderTargetsInvertedY() {
	return true;
}

bool Graphics::nonPow2TexturesSupported() {
	return false;
}

void Graphics::flush() {

}
