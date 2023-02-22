#include "kinc/graphics4/graphics.h"
#include "vulkan.h"

#include <kinc/error.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

struct vk_funs vk = {0};
struct vk_context vk_ctx = {0};

#ifdef KORE_WINDOWS
#define ERR_EXIT(err_msg, err_class)                                                                                                                           \
	do {                                                                                                                                                       \
		MessageBoxA(NULL, err_msg, err_class, MB_OK);                                                                                                          \
		exit(1);                                                                                                                                               \
	} while (0)
#else
#define ERR_EXIT(err_msg, err_class)                                                                                                                           \
	do {                                                                                                                                                       \
		kinc_log(KINC_LOG_LEVEL_ERROR, "%s", err_msg);                                                                                                         \
		exit(1);                                                                                                                                               \
	} while (0)
#endif

void kinc_vulkan_get_instance_extensions(const char **extensions, int *index, int max);
VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
VkResult kinc_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface);

#define GET_INSTANCE_PROC_ADDR(instance, entrypoint)                                                                                                           \
	{                                                                                                                                                          \
		vk.fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(instance, "vk" #entrypoint);                                                             \
		if (vk.fp##entrypoint == NULL) {                                                                                                                       \
			ERR_EXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint, "vkGetInstanceProcAddr Failure");                                                  \
		}                                                                                                                                                      \
	}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define APP_NAME_STR_LEN 80

void createDescriptorLayout(void);
void set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

// uint32_t current_buffer;

kinc_g5_texture_t *vulkanTextures[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
kinc_g5_render_target_t *vulkanRenderTargets[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
VkSampler vulkanSamplers[16] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
                                VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};

static bool began = false;

#ifndef KORE_ANDROID
static VkAllocationCallbacks allocator;
#endif

static VkPhysicalDeviceProperties gpu_props;
static VkQueueFamilyProperties *queue_props;
static uint32_t graphics_queue_node_index;

static VkPhysicalDeviceMemoryProperties memory_properties;

static uint32_t queue_count;

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

void kinc_g5_internal_destroy_window(int window) {}

void kinc_g5_internal_destroy() {}

extern void kinc_g4_on_g5_internal_resize(int, int, int);

void kinc_internal_resize(int window_index, int width, int height) {
	struct vk_window *window = &vk_ctx.windows[window_index];
	window->new_width = width;
	window->new_height = height;

	kinc_g4_on_g5_internal_resize(window_index, width, height);
}

void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {}

void create_swapchain(struct vk_window *window) {
	VkSwapchainKHR oldSwapchain = window->swapchain;

	// Check the surface capabilities and formats
	VkSurfaceCapabilitiesKHR surfCapabilities = {0};
	VkResult err = vk.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_ctx.gpu, window->surface, &surfCapabilities);
	assert(!err);

	uint32_t presentModeCount;
	err = vk.fpGetPhysicalDeviceSurfacePresentModesKHR(vk_ctx.gpu, window->surface, &presentModeCount, NULL);
	assert(!err);
	VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	assert(presentModes);
	err = vk.fpGetPhysicalDeviceSurfacePresentModesKHR(vk_ctx.gpu, window->surface, &presentModeCount, presentModes);
	assert(!err);

	VkExtent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = window->width;
		swapchainExtent.height = window->height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		window->width = surfCapabilities.currentExtent.width;
		window->height = surfCapabilities.currentExtent.height;
	}

	VkPresentModeKHR swapchainPresentMode = window->vsynced ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;

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
	swapchain_info.surface = window->surface;
	swapchain_info.minImageCount = desiredNumberOfSwapchainImages;
	swapchain_info.imageFormat = window->format.format;
	swapchain_info.imageColorSpace = window->format.colorSpace;
	swapchain_info.imageExtent.width = swapchainExtent.width;
	swapchain_info.imageExtent.height = swapchainExtent.height;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.preTransform = preTransform;

	if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	else if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	else if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}
	else {
		kinc_log(KINC_LOG_LEVEL_ERROR, "No supported composite alpha, this should not happen.\nPlease go complain to the writers of your Vulkan driver.");
		exit(1);
	}

	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.queueFamilyIndexCount = 0;
	swapchain_info.pQueueFamilyIndices = NULL;
	swapchain_info.presentMode = swapchainPresentMode;
	swapchain_info.oldSwapchain = oldSwapchain;
	swapchain_info.clipped = true;

	err = vk.fpCreateSwapchainKHR(vk_ctx.device, &swapchain_info, NULL, &window->swapchain);
	assert(!err);

	// If we just re-created an existing swapchain, we should destroy the old
	// swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated
	// presentable images once the platform is done with them.
	if (oldSwapchain != VK_NULL_HANDLE) {
		vk.fpDestroySwapchainKHR(vk_ctx.device, oldSwapchain, NULL);
	}

	err = vk.fpGetSwapchainImagesKHR(vk_ctx.device, window->swapchain, &window->image_count, NULL);
	assert(!err);

	if (window->images) {
		free(window->images);
		window->images = NULL;
	}

	if (window->views) {
		free(window->views);
		window->views = NULL;
	}

	window->images = (VkImage *)malloc(window->image_count * sizeof(VkImage));
	assert(window->images);

	err = vk.fpGetSwapchainImagesKHR(vk_ctx.device, window->swapchain, &window->image_count, window->images);
	assert(!err);

	window->views = (VkImageView *)malloc(window->image_count * sizeof(VkImageView));
	assert(window->views);

	for (uint32_t i = 0; i < window->image_count; i++) {
		VkImageViewCreateInfo color_attachment_view = {0};
		color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_attachment_view.pNext = NULL;
		color_attachment_view.format = window->format.format;
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

		// Render loop will expect image to have been used before and in
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		// layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
		// to that state
		set_image_layout(window->images[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		color_attachment_view.image = window->images[i];

		err = vkCreateImageView(vk_ctx.device, &color_attachment_view, NULL, &window->views[i]);
		assert(!err);
	}

	window->current_image = 0;

	if (NULL != presentModes) {
		free(presentModes);
	}

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;

	if (window->depth_bits > 0) {
		VkImageCreateInfo image = {0};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = depth_format;
		image.extent.width = window->width;
		image.extent.height = window->height;
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
		err = vkCreateImage(vk_ctx.device, &image, NULL, &window->depth.image);
		assert(!err);

		/* get memory requirements for this object */
		vkGetImageMemoryRequirements(vk_ctx.device, window->depth.image, &mem_reqs);

		/* select memory size and type */
		mem_alloc.allocationSize = mem_reqs.size;
		pass = memory_type_from_properties(mem_reqs.memoryTypeBits, 0, /* No requirements */ &mem_alloc.memoryTypeIndex);
		assert(pass);

		/* allocate memory */
		err = vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &window->depth.memory);
		assert(!err);

		/* bind memory */
		err = vkBindImageMemory(vk_ctx.device, window->depth.image, window->depth.memory, 0);
		assert(!err);

		set_image_layout(window->depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		/* create image view */
		view.image = window->depth.image;
		err = vkCreateImageView(vk_ctx.device, &view, NULL, &window->depth.view);
		assert(!err);
	}

	VkAttachmentDescription attachments[2];
	attachments[0].format = window->format.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	if (window->depth_bits > 0) {
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
	subpass.pDepthStencilAttachment = window->depth_bits > 0 ? &depth_reference : NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkSubpassDependency dependencies[2];
	memset(&dependencies, 0, sizeof(dependencies));

	// for the frame-buffer
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = window->depth_bits > 0 ? 2 : 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 2;
	rp_info.pDependencies = dependencies;

	err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->framebuffer_render_pass);
	assert(!err);

	VkImageView attachmentViews[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};

	if (window->depth_bits > 0) {
		attachmentViews[1] = window->depth.view;
	}

	VkFramebufferCreateInfo fb_info = {0};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = window->framebuffer_render_pass;
	fb_info.attachmentCount = window->depth_bits > 0 ? 2 : 1;
	fb_info.pAttachments = attachmentViews;
	fb_info.width = window->width;
	fb_info.height = window->height;
	fb_info.layers = 1;

	if (window->framebuffers) {
		free(window->framebuffers);
		window->framebuffers = NULL;
	}

	window->framebuffers = (VkFramebuffer *)malloc(window->image_count * sizeof(VkFramebuffer));
	assert(window->framebuffers);

	for (uint32_t i = 0; i < window->image_count; i++) {
		attachmentViews[0] = window->views[i];
		err = vkCreateFramebuffer(vk_ctx.device, &fb_info, NULL, &window->framebuffers[i]);
		assert(!err);
	}

	flush_init_cmd();
}

void create_render_target_render_pass(struct vk_window *window) {
	VkAttachmentDescription attachments[2];
	attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM; // target->impl.format; // TODO
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	// TODO
	/*if (depthBufferBits > 0) {
	    attachments[1].format = VK_FORMAT_D16_UNORM;
	    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	    attachments[1].flags = 0;
	}*/

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
	subpass.pDepthStencilAttachment = /*depthBufferBits > 0 ? &depth_reference :*/ NULL; // TODO
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	// for render-targets
	VkSubpassDependency dependencies[2];
	memset(&dependencies, 0, sizeof(dependencies));

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = /*depthBufferBits > 0 ? 2 :*/ 1; // TODO
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 2;
	rp_info.pDependencies = dependencies;

	VkResult err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->rendertarget_render_pass);
	assert(!err);
}

static bool check_extensions(const char **wanted_extensions, int wanted_extension_count, VkExtensionProperties *extensions, int extension_count) {
	bool *found_extensions = malloc(wanted_extension_count);

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

static bool find_layer(VkLayerProperties *layers, int layer_count, const char *wanted_layer) {
	for (int i = 0; i < layer_count; i++) {
		if (strcmp(wanted_layer, layers[i].layerName) == 0) {
			return true;
		}
	}

	return false;
}

void kinc_g5_internal_init() {
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

#ifdef VALIDATE
	wanted_instance_extensions[wanted_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif
	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
	kinc_vulkan_get_instance_extensions(wanted_instance_extensions, &wanted_instance_extension_count, ARRAY_SIZE(wanted_instance_extensions));

	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	assert(!err);
	VkExtensionProperties *instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
	assert(!err);
	bool missing_instance_extensions =
	    check_extensions(wanted_instance_extensions, wanted_instance_extension_count, instance_extensions, instance_extension_count);

	if (missing_instance_extensions) {
		exit(1);
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

#ifndef KORE_ANDROID
	allocator.pfnAllocation = myalloc;
	allocator.pfnFree = myfree;
	allocator.pfnReallocation = myrealloc;
	err = vkCreateInstance(&info, &allocator, &vk_ctx.instance);
#else
	err = vkCreateInstance(&info, NULL, &vk_ctx.instance);
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

	uint32_t gpu_count;

	err = vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, NULL);
	assert(!err && gpu_count > 0);

	if (gpu_count > 0) {
		VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, physical_devices);
		assert(!err);
		// TODO: expose gpu selection to user?
		vk_ctx.gpu = physical_devices[0];
		free(physical_devices);
	}
	else {
		ERR_EXIT("vkEnumeratePhysicalDevices reported zero accessible devices."
		         "\n\nDo you have a compatible Vulkan installable client"
		         " driver (ICD) installed?\nPlease look at the Getting Started"
		         " guide for additional information.\n",
		         "vkEnumeratePhysicalDevices Failure");
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

#ifdef KORE_VKRT
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
#endif

	uint32_t device_extension_count;

	err = vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, NULL);
	assert(!err);

	VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
	err = vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, device_extensions);
	assert(!err);

	bool missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);

	free(device_extensions);

	if (missing_device_extensions) {
		exit(1);
	}

#ifdef VALIDATE
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
#endif

	// Having these GIPA queries of vk_ctx.device extension entry points both
	// BEFORE and AFTER vkCreateDevice is a good test for the loader
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, CreateSwapchainKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, DestroySwapchainKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetSwapchainImagesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, AcquireNextImageKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, QueuePresentKHR);

	vkGetPhysicalDeviceProperties(vk_ctx.gpu, &gpu_props);

	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(vk_ctx.gpu, &queue_count, NULL);

	queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(vk_ctx.gpu, &queue_count, queue_props);
	assert(queue_count >= 1);

	bool headless = false;

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

			deviceinfo.enabledLayerCount = wanted_device_layer_count;
			deviceinfo.ppEnabledLayerNames = (const char *const *)wanted_device_layers;

			deviceinfo.enabledExtensionCount = wanted_device_extension_count;
			deviceinfo.ppEnabledExtensionNames = (const char *const *)wanted_device_extensions;

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

		createDescriptorLayout();
		assert(!err);
	}

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {0};
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	err = vkCreateSemaphore(vk_ctx.device, &presentCompleteSemaphoreCreateInfo, NULL, &framebuffer_available);
	assert(!err);
}

void kinc_g5_internal_init_window(int window_index, int depthBufferBits, int stencilBufferBits, bool vsync) {
	assert(window_index < MAXIMUM_WINDOWS);
	struct vk_window *window = &vk_ctx.windows[window_index];

	window->depth_bits = depthBufferBits;
	window->stencil_bits = stencilBufferBits;
	window->vsynced = vsync;

	VkResult err = kinc_vulkan_create_surface(vk_ctx.instance, window_index, &window->surface);
	assert(!err);

	uint32_t formatCount;
	err = vk.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_ctx.gpu, window->surface, &formatCount, NULL);
	assert(!err);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	err = vk.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_ctx.gpu, window->surface, &formatCount, surfFormats);
	assert(!err);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		window->format = surfFormats[0];
	}
	else {
		assert(formatCount >= 1);
		window->format = surfFormats[0];
	}
	window->width = window->new_width = kinc_window_width(window_index);
	window->height = window->new_height = kinc_window_height(window_index);
	create_swapchain(window);
	create_render_target_render_pass(window);

	began = false;
	kinc_g5_begin(NULL, 0);
}

bool kinc_window_vsynced(int window) {
	return true;
}

bool kinc_g5_swap_buffers() {
	return true;
}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window_index) {
	struct vk_window *window = &vk_ctx.windows[window_index];
	if (renderTarget != NULL) {
		renderTarget->impl.framebuffer = window->framebuffers[window->current_image];
	}

	if (began)
		return;

	if (window->new_width != window->width || window->new_height != window->height) {
		window->width = window->new_width;
		window->height = window->new_height;
		vkDeviceWaitIdle(vk_ctx.device);
		create_swapchain(window);
	}

	// Get the index of the next available swapchain image:
	command_list_should_wait_for_framebuffer();
	VkResult err = vk.fpAcquireNextImageKHR(vk_ctx.device, window->swapchain, UINT64_MAX, framebuffer_available, VK_NULL_HANDLE, &window->current_image);
	// TODO: handle this somehow, currently this will just crash on Android
	if (err == VK_ERROR_SURFACE_LOST_KHR) {
		kinc_g5_internal_destroy_window(window_index);
		kinc_g5_internal_init_window(window_index, window->depth_bits, window->stencil_bits, window->vsynced);
	}
	else {
		assert(!err);
		began = true;
		vk_ctx.current_window = window_index;
	}
}

void kinc_g5_end(int window) {
	VkPresentInfoKHR present = {0};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &vk_ctx.windows[vk_ctx.current_window].swapchain;
	present.pImageIndices = &vk_ctx.windows[vk_ctx.current_window].current_image;

	VkResult err = vk.fpQueuePresentKHR(vk_ctx.queue, &present);
	assert(err == VK_SUCCESS || err == VK_ERROR_OUT_OF_DATE_KHR);

	reuse_descriptor_sets();
	began = false;
}

void kinc_g5_flush() {}

// this is exclusively used by the Android backend at the moment
bool kinc_vulkan_internal_get_size(int *width, int *height) {
	if (vk_ctx.windows[0].surface) {
		VkSurfaceCapabilitiesKHR capabilities;
		VkResult err = vk.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_ctx.gpu, vk_ctx.windows[0].surface, &capabilities);
		assert(!err);
		*width = capabilities.currentExtent.width;
		*height = capabilities.currentExtent.height;
		return true;
	}
	else {
		return false;
	}
}

bool kinc_g5_supports_raytracing() {
#ifdef KORE_VKRT
	return true;
#else
	return false;
#endif
}

bool kinc_g5_supports_instanced_rendering() {
	return true;
}

bool kinc_g5_supports_compute_shaders() {
	return true;
}

bool kinc_g5_supports_blend_constants() {
	return true;
}

bool kinc_g5_supports_non_pow2_textures() {
	return true;
}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}

int kinc_g5_max_bound_textures(void) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(vk_ctx.gpu, &props);
	return props.limits.maxPerStageDescriptorSamplers;
}
