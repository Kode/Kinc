#include <kinc/graphics6/texture.h>

#include "vulkan.h"

VkImageType convert_image_type(kinc_g6_texture_dimension_t dimension) {
	switch (dimension) {
	case KINC_G6_TEXTURE_DIMENSION_1D:
		return VK_IMAGE_TYPE_1D;
	case KINC_G6_TEXTURE_DIMENSION_2D:
		return VK_IMAGE_TYPE_2D;
	case KINC_G6_TEXTURE_DIMENSION_3D:
		return VK_IMAGE_TYPE_3D;
	}
}

VkFormat convert_texture_format(kinc_g6_texture_format_t format);

VkImageUsageFlags convert_usage(kinc_g6_texture_usage_t usage) {
	VkImageUsageFlags image_usage = 0;
	if (usage & KINC_G6_TEXTURE_USAGE_COPY_SRC) image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (usage & KINC_G6_TEXTURE_USAGE_COPY_DST) image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (usage & KINC_G6_TEXTURE_USAGE_TEXTURE_BINDING) image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (usage & KINC_G6_TEXTURE_USAGE_STORAGE_BINDING) image_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	// TODO : depth & stencil
	if (usage & KINC_G6_TEXTURE_USAGE_RENDER_ATTACHMENT) image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}
void kinc_g6_texture_init(kinc_g6_texture_t *texture, const kinc_g6_texture_descriptor_t *descriptor) {
	VkImageCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.imageType = convert_image_type(descriptor->dimension);
	create_info.format = texture->impl.format = convert_texture_format(descriptor->format);
	create_info.extent.width = texture->impl.extent.width = descriptor->width;
	create_info.extent.height = texture->impl.extent.height = descriptor->height;
	create_info.extent.depth = texture->impl.extent.depth = descriptor->depth;
	create_info.mipLevels = descriptor->mipLevels;
	create_info.arrayLayers = descriptor->arrayLayers;
	create_info.samples = descriptor->sampleCount;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.usage = convert_usage(descriptor->usage);
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = NULL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkCreateImage(context.device, &create_info, NULL, &texture->impl.image);
}

void kinc_g6_texture_destroy(kinc_g6_texture_t *texture) {
	vkDestroyImage(context.device, texture->impl.image, NULL);
}

VkImageViewType convert_texture_view_dimension(kinc_g6_texture_view_dimension_t dimension) {
	switch (dimension) {
	case KINC_G6_TEXTURE_VIEW_DIMENSION_1D:
		return VK_IMAGE_VIEW_TYPE_1D;
	case KINC_G6_TEXTURE_VIEW_DIMENSION_2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case KINC_G6_TEXTURE_VIEW_DIMENSION_3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	case KINC_G6_TEXTURE_VIEW_DIMENSION_CUBE:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case KINC_G6_TEXTURE_VIEW_DIMENSION_1D_ARRAY:
		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case KINC_G6_TEXTURE_VIEW_DIMENSION_2D_ARRAY:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case KINC_G6_TEXTURE_VIEW_DIMENSION_CUBE_ARRAY:
		return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	}
}

VkImageAspectFlags convert_aspect(kinc_g6_texture_aspect_t aspect) {
	switch (aspect) {
	case KINC_G6_TEXTURE_ASPECT_ALL:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	case KINC_G6_TEXTURE_ASPECT_STENCIL_ONLY:
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	case KINC_G6_TEXTURE_ASPECT_DEPTH_ONLY:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
}

void kinc_g6_texture_view_init(kinc_g6_texture_view_t *view, const kinc_g6_texture_view_descriptor_t *descriptor) {
	VkImageViewCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.image = descriptor->texture->impl.image;
	create_info.viewType = convert_texture_view_dimension(descriptor->dimension);
	create_info.format = view->impl.format =
	    descriptor->format == KINC_G6_TEXTURE_FORMAT_NONE ? descriptor->texture->impl.format : convert_texture_format(descriptor->format);
	create_info.components = (VkComponentMapping){
	    .r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY};
	create_info.subresourceRange.aspectMask = convert_aspect(descriptor->aspect);
	create_info.subresourceRange.baseMipLevel = descriptor->base_mip_level;
	create_info.subresourceRange.levelCount = descriptor->mip_level_count;
	create_info.subresourceRange.baseArrayLayer = descriptor->base_array_layer;
	create_info.subresourceRange.layerCount = descriptor->array_layer_count;
	view->impl.extent.width = descriptor->texture->impl.extent.width >> descriptor->base_mip_level;
	view->impl.extent.height = descriptor->texture->impl.extent.height >> descriptor->base_mip_level;
	view->impl.extent.depth = 0;
	vkCreateImageView(context.device, &create_info, NULL, &view->impl.view);
}
void kinc_g6_texture_view_destroy(kinc_g6_texture_view_t *view) {
	vkDestroyImageView(context.device, view->impl.view, NULL);
}