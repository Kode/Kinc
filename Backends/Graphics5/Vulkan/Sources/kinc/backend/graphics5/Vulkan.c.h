#include <kinc/error.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/system.h>
#include <kinc/window.h>

#ifdef KORE_WINDOWS
#define ERR_EXIT(err_msg, err_class)                                                                                                                           \
	do {                                                                                                                                                       \
		MessageBoxA(NULL, err_msg, err_class, MB_OK);                                                                                                          \
		exit(1);                                                                                                                                               \
	} while (0)
#else
#define ERR_EXIT(err_msg, err_class)                                                                                                                           \
	do {                                                                                                                                                       \
		printf(err_msg);                                                                                                                                       \
		fflush(stdout);                                                                                                                                        \
		exit(1);                                                                                                                                               \
	} while (0)
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#define KINC_SURFACE_EXT_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
#define KINC_SURFACE_EXT_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define KINC_SURFACE_EXT_NAME VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif
VkResult kinc_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface);

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                                                                               \
	{                                                                                                                                                          \
		fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint);                                                                    \
		if (fp##entrypoint == NULL) {                                                                                                                          \
			ERR_EXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint, "vkGetInstanceProcAddr Failure");                                                  \
		}                                                                                                                                                      \
	}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define APP_NAME_STR_LEN 80

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

VkDevice device;
VkFormat format;
VkRenderPass render_pass;
VkPhysicalDevice gpu;
VkCommandPool cmd_pool;
VkQueue queue;
uint32_t swapchainImageCount;
VkFramebuffer *framebuffers;
PFN_vkQueuePresentKHR fpQueuePresentKHR;
PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
VkSemaphore presentCompleteSemaphore;
void createDescriptorLayout(void);
void set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

#ifdef _DEBUG
#define VALIDATE
#endif

struct SwapchainBuffers *kinc_vulkan_internal_buffers;
struct DepthBuffer kinc_vulkan_internal_depth;
VkSwapchainKHR swapchain;
uint32_t current_buffer;
int depthBits;
int stencilBits;
bool vsynced;

kinc_g5_texture_t *vulkanTextures[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
kinc_g5_render_target_t *vulkanRenderTargets[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static bool began = false;
#ifdef KORE_WINDOWS
static HWND windowHandle;

static HINSTANCE connection;        // hInstance - Windows Instance
static char name[APP_NAME_STR_LEN]; // Name to put on the window/icon
static HWND window;                 // hWnd - window handle
#endif
static VkSurfaceKHR surface;
static bool prepared;
#ifndef KORE_ANDROID
static VkAllocationCallbacks allocator;
#endif
static VkInstance inst;
static VkPhysicalDeviceProperties gpu_props;
static VkQueueFamilyProperties *queue_props;
static uint32_t graphics_queue_node_index;

static uint32_t enabled_extension_count;
#ifdef VALIDATE
static uint32_t enabled_layer_count;
#endif
static char *extension_names[64];
#ifdef VALIDATE
static char *device_validation_layers[64];
#endif

static VkColorSpaceKHR color_space;

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
static PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
static PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
static PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;

static VkPhysicalDeviceMemoryProperties memory_properties;

static PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
static PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
static VkDebugReportCallbackEXT msg_callback;

static bool quit;
static uint32_t queue_count;

static VkBool32 check_layers(uint32_t check_count, char **check_names, uint32_t layer_count, VkLayerProperties *layers) {
	for (uint32_t i = 0; i < check_count; ++i) {
		VkBool32 found = 0;
		for (uint32_t j = 0; j < layer_count; ++j) {
			if (!strcmp(check_names[i], layers[j].layerName)) {
				found = 1;
				break;
			}
		}
		if (!found) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Cannot find layer: %s\n", check_names[i]);
			return 0;
		}
	}
	return 1;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode,
                                              const char *pLayerPrefix, const char *pMsg, void *pUserData) {
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
	}
	return false;
}
#ifndef KORE_ANDROID
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
	return aligned_alloc(alignment, size);
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
static int powi(int power) {
	int ret = 1;
	for (int i = 0; i < power; ++i) ret *= 2;
	return ret;
}

static int getPower2(int i) {
	for (int power = 0;; ++power)
		if (powi(power) >= i) return powi(power);
}

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

void kinc_g5_destroy(int window) {}

void kinc_internal_g5_resize(int window, int width, int height) {}

void kinc_internal_resize(int window, int width, int height) {
	if (width == 0 || height == 0) return;
	newRenderTargetWidth = width;
	newRenderTargetHeight = height;
}

void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {}

void create_swapchain() {
	VkSwapchainKHR oldSwapchain = swapchain;

	// Check the surface capabilities and formats
	VkSurfaceCapabilitiesKHR surfCapabilities = {0};
	VkResult err = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfCapabilities);
	assert(!err);

	uint32_t presentModeCount;
	err = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, NULL);
	assert(!err);
	VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	assert(presentModes);
	err = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes);
	assert(!err);

	VkExtent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = renderTargetWidth;
		swapchainExtent.height = renderTargetHeight;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		renderTargetWidth = surfCapabilities.currentExtent.width;
		renderTargetHeight = surfCapabilities.currentExtent.height;
	}

	VkPresentModeKHR swapchainPresentMode = vsynced ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform = {0};
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchain_info = {0};
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

	VkImage *swapchainImages = (VkImage *)malloc(swapchainImageCount * sizeof(VkImage));
	assert(swapchainImages);
	err = fpGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);
	assert(!err);

	kinc_vulkan_internal_buffers = (struct SwapchainBuffers *)malloc(sizeof(struct SwapchainBuffers) * swapchainImageCount);
	assert(kinc_vulkan_internal_buffers != NULL);

	for (i = 0; i < swapchainImageCount; i++) {
		VkImageViewCreateInfo color_attachment_view = {0};
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

		kinc_vulkan_internal_buffers[i].image = swapchainImages[i];

		// Render loop will expect image to have been used before and in
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		// layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
		// to that state
		set_image_layout(kinc_vulkan_internal_buffers[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		color_attachment_view.image = kinc_vulkan_internal_buffers[i].image;

		err = vkCreateImageView(device, &color_attachment_view, NULL, &kinc_vulkan_internal_buffers[i].view);
		assert(!err);
	}

	current_buffer = 0;

	if (NULL != presentModes) {
		free(presentModes);
	}

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;

	if (depthBits > 0) {
		VkImageCreateInfo image = {0};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = depth_format;
		image.extent.width = renderTargetWidth;
		image.extent.height = renderTargetHeight;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image.flags = 0;

		VkMemoryAllocateInfo mem_alloc = {0};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkImageViewCreateInfo view = {0};
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

		VkMemoryRequirements mem_reqs = {0};
		bool pass;

		/* create image */
		err = vkCreateImage(device, &image, NULL, &kinc_vulkan_internal_depth.image);
		assert(!err);

		/* get memory requirements for this object */
		vkGetImageMemoryRequirements(device, kinc_vulkan_internal_depth.image, &mem_reqs);

		/* select memory size and type */
		mem_alloc.allocationSize = mem_reqs.size;
		pass = memory_type_from_properties(mem_reqs.memoryTypeBits, 0, /* No requirements */ &mem_alloc.memoryTypeIndex);
		assert(pass);

		/* allocate memory */
		err = vkAllocateMemory(device, &mem_alloc, NULL, &kinc_vulkan_internal_depth.mem);
		assert(!err);

		/* bind memory */
		err = vkBindImageMemory(device, kinc_vulkan_internal_depth.image, kinc_vulkan_internal_depth.mem, 0);
		assert(!err);

		set_image_layout(kinc_vulkan_internal_depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		/* create image view */
		view.image = kinc_vulkan_internal_depth.image;
		err = vkCreateImageView(device, &view, NULL, &kinc_vulkan_internal_depth.view);
		assert(!err);
	}

	VkAttachmentDescription attachments[2];
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	if (depthBits > 0) {
		attachments[1].format = depth_format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;
	}

	VkAttachmentReference color_reference = {0};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {0};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = depthBits > 0 ? &depth_reference : NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = depthBits > 0 ? 2 : 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	err = vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
	assert(!err);

	VkImageView attachmentViews[2];

	if (depthBits > 0) {
		attachmentViews[1] = kinc_vulkan_internal_depth.view;
	}

	VkFramebufferCreateInfo fb_info = {0};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = render_pass;
	fb_info.attachmentCount = depthBits > 0 ? 2 : 1;
	fb_info.pAttachments = attachmentViews;
	fb_info.width = renderTargetWidth;
	fb_info.height = renderTargetHeight;
	fb_info.layers = 1;

	framebuffers = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));
	assert(framebuffers);

	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		attachmentViews[0] = kinc_vulkan_internal_buffers[i].view;
		err = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);
		assert(!err);
	}
}

void kinc_g5_init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	// depthBits = depthBufferBits;
	depthBits = 0;
	stencilBits = stencilBufferBits;
	vsynced = vsync;
	uint32_t instance_extension_count = 0;
	uint32_t instance_layer_count = 0;
#ifdef VALIDATE
	uint32_t device_validation_layer_count = 0;
#endif

#ifdef VALIDATE
	char *instance_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
#endif

#ifdef VALIDATE
	device_validation_layers[0] = "VK_LAYER_KHRONOS_validation";
	device_validation_layer_count = 1;
#endif

	VkBool32 validation_found = 0;
	VkResult err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
	assert(!err);

	if (instance_layer_count > 0) {
		VkLayerProperties *instance_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
		assert(!err);

#ifdef VALIDATE
		validation_found = check_layers(ARRAY_SIZE(instance_validation_layers), instance_validation_layers, instance_layer_count, instance_layers);
		enabled_layer_count = ARRAY_SIZE(instance_validation_layers);
#endif
		free(instance_layers);
	}

	// Look for instance extensions
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
	memset(extension_names, 0, sizeof(extension_names));

	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	assert(!err);

	if (instance_extension_count > 0) {
		VkExtensionProperties *instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
		err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
		assert(!err);
		for (uint32_t i = 0; i < instance_extension_count; i++) {
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
				surfaceExtFound = 1;
				extension_names[enabled_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}
			if (!strcmp(KINC_SURFACE_EXT_NAME, instance_extensions[i].extensionName)) {
				platformSurfaceExtFound = 1;
				extension_names[enabled_extension_count++] = KINC_SURFACE_EXT_NAME;
			}
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
		         "the " VK_KHR_SURFACE_EXTENSION_NAME " extension.\n\nDo you have a compatible "
		         "Vulkan installable client driver (ICD) installed?\nPlease "
		         "look at the Getting Started guide for additional "
		         "information.\n",
		         "vkCreateInstance Failure");
	}
	if (!platformSurfaceExtFound) {
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
		         "the " KINC_SURFACE_EXT_NAME " extension.\n\nDo you have a compatible "
		         "Vulkan installable client driver (ICD) installed?\nPlease "
		         "look at the Getting Started guide for additional "
		         "information.\n",
		         "vkCreateInstance Failure");
	}
	VkApplicationInfo app = {0};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL;
	app.pApplicationName = kinc_application_name();
	app.applicationVersion = 0;
	app.pEngineName = "Kore";
	app.engineVersion = 0;
#ifdef KORE_VKRT
	app.apiVersion = VK_API_VERSION_1_2;
#else
	app.apiVersion = VK_API_VERSION_1_0;
#endif

	VkInstanceCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.pApplicationInfo = &app;
#ifdef VALIDATE
	info.enabledLayerCount = enabled_layer_count;
	info.ppEnabledLayerNames = (const char *const *)instance_validation_layers;
#else
	info.enabledLayerCount = 0;
	info.ppEnabledLayerNames = NULL;
#endif
	info.enabledExtensionCount = enabled_extension_count;
	info.ppEnabledExtensionNames = (const char *const *)extension_names;

	uint32_t gpu_count;

#ifndef KORE_ANDROID
	allocator.pfnAllocation = myalloc;
	allocator.pfnFree = myfree;
	allocator.pfnReallocation = myrealloc;
	err = vkCreateInstance(&info, &allocator, &inst);
#else
	err = vkCreateInstance(&info, NULL, &inst);
#endif
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
		VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
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
		VkLayerProperties *device_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * device_layer_count);
		err = vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layers);
		assert(!err);

#ifdef VALIDATE
		validation_found = check_layers(device_validation_layer_count, device_validation_layers, device_layer_count, device_layers);
		enabled_layer_count = device_validation_layer_count;
#endif

		free(device_layers);
	}

	/* Loog for device extensions */
	uint32_t device_extension_count = 0;
	VkBool32 swapchainExtFound = 0;
	enabled_extension_count = 0;
	memset(extension_names, 0, sizeof(extension_names));

	err = vkEnumerateDeviceExtensionProperties(gpu, NULL, &device_extension_count, NULL);
	assert(!err);

	if (device_extension_count > 0) {
		VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
		err = vkEnumerateDeviceExtensionProperties(gpu, NULL, &device_extension_count, device_extensions);
		assert(!err);

		for (uint32_t i = 0; i < device_extension_count; i++) {
			if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
				swapchainExtFound = 1;
				extension_names[enabled_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}
			if (!strcmp(VK_KHR_MAINTENANCE1_EXTENSION_NAME, device_extensions[i].extensionName)) {
				// Allows negative viewport height to flip viewport
				extension_names[enabled_extension_count++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
			}
#ifdef KORE_VKRT
			if (!strcmp(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
			}
			if (!strcmp(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
			}
			if (!strcmp(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
			}
			if (!strcmp(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
			}
			if (!strcmp(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
			}
			if (!strcmp(VK_KHR_SPIRV_1_4_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
			}
			if (!strcmp(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, device_extensions[i].extensionName)) {
				extension_names[enabled_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
			}
#endif
			assert(enabled_extension_count < 64);
		}

		free(device_extensions);
	}

	if (!swapchainExtFound) {
		ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find "
		         "the " VK_KHR_SWAPCHAIN_EXTENSION_NAME " extension.\n\nDo you have a compatible "
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
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {0};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
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

	queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);
	assert(queue_count >= 1);

	renderTargetWidth = kinc_window_width(window);
	renderTargetHeight = kinc_window_height(window);

	{
		VkResult err;
		uint32_t i;
		err = kinc_vulkan_create_surface(inst, window, &surface);
		assert(!err);

		// Iterate over each queue to learn whether it supports presenting:
		VkBool32 *supportsPresent = (VkBool32 *)malloc(queue_count * sizeof(VkBool32));
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
		if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
			ERR_EXIT("Could not find a graphics and a present queue\n", "Swapchain Initialization Failure");
		}

		// TODO: Add support for separate queues, including presentation,
		//       synchronization, and appropriate tracking for QueueSubmit.
		// NOTE: While it is possible for an application to use a separate graphics
		//       and a present queues, this demo program assumes it is only using
		//       one:
		if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
			ERR_EXIT("Could not find a common graphics and a present queue\n", "Swapchain Initialization Failure");
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
#ifdef VALIDATE
			deviceinfo.enabledLayerCount = enabled_layer_count;
			deviceinfo.ppEnabledLayerNames = (const char *const *)device_validation_layers;
#else
			deviceinfo.enabledLayerCount = 0;
			deviceinfo.ppEnabledLayerNames = NULL;
#endif
			deviceinfo.enabledExtensionCount = enabled_extension_count;
			deviceinfo.ppEnabledExtensionNames = (const char *const *)extension_names;

#ifdef KORE_VKRT
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

			err = vkCreateDevice(gpu, &deviceinfo, NULL, &device);
			assert(!err);
		}

		vkGetDeviceQueue(device, graphics_queue_node_index, 0, &queue);

		// Get the list of VkFormat's that are supported:
		uint32_t formatCount;
		err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, NULL);
		assert(!err);
		VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
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

	VkCommandPoolCreateInfo cmd_pool_info = {0};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = graphics_queue_node_index;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	err = vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmd_pool);
	assert(!err);

	create_swapchain();

	createDescriptorLayout();

	began = false;
	kinc_g5_begin(NULL, 0);
}

bool kinc_window_vsynced(int window) {
	return true;
}

// void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {
// 	// drawIndexedVerticesInstanced(instanceCount, 0, IndexBufferImpl::current->count());
// }

// void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

bool kinc_g5_swap_buffers() {
	return true;
}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {
	if (renderTarget != NULL) {
		renderTarget->impl.renderPass = render_pass;
		renderTarget->impl.framebuffer = framebuffers[current_buffer];
	}

	if (began) return;

	if (newRenderTargetWidth != renderTargetWidth || newRenderTargetHeight != renderTargetHeight) {
		renderTargetWidth = newRenderTargetWidth;
		renderTargetHeight = newRenderTargetHeight;
		vkDeviceWaitIdle(device);
		create_swapchain();
	}

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {0};
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	VkResult err = vkCreateSemaphore(device, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);
	assert(!err);

	// Get the index of the next available swapchain image:
	err = fpAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)0, // TODO: Show use of fence
	                            &current_buffer);

	began = true;
}

void kinc_g5_end(int window) {
	began = false;
}

void kinc_g5_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	assert(unit.impl.binding >= 2); // Make sure the spirv-bindings have been read correctly
	vulkanTextures[unit.impl.binding - 2] = texture;
	vulkanRenderTargets[unit.impl.binding - 2] = NULL;
}

void kinc_g5_set_image_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

void kinc_g5_set_texture_operation(kinc_g5_texture_operation_t operation, kinc_g5_texture_argument_t arg1, kinc_g5_texture_argument_t arg2) {}

void kinc_g5_set_render_target_face(kinc_g5_render_target_t *texture, int face) {}

int kinc_g5_max_bound_textures(void) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(gpu, &props);
	return props.limits.maxPerStageDescriptorSamplers;
}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}

bool kinc_g5_non_pow2_textures_qupported() {
	return true;
}

void kinc_g5_flush() {}

bool kinc_g5_init_occlusion_query(unsigned *occlusionQuery) {
	return false;
}

void kinc_g5_delete_occlusion_query(unsigned occlusionQuery) {}

void kinc_g5_render_occlusion_query(unsigned occlusionQuery, int triangles) {}

bool kinc_g5_are_query_results_available(unsigned occlusionQuery) {
	return false;
}

void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount) {}
