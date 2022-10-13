#include "vulkan.h"
#include <kinc/graphics5/sampler.h>
#include <vulkan/vulkan_core.h>

static VkCompareOp convert_compare_mode(kinc_g5_compare_mode_t compare);
static VkSamplerAddressMode convert_addressing(kinc_g5_texture_addressing_t mode) {
	switch (mode) {
	case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case KINC_G5_TEXTURE_ADDRESSING_BORDER:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	}
}

static VkSamplerMipmapMode convert_mipmap_mode(kinc_g5_mipmap_filter_t filter) {
	switch (filter) {
	case KINC_G5_MIPMAP_FILTER_NONE:
	case KINC_G5_MIPMAP_FILTER_POINT:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case KINC_G5_MIPMAP_FILTER_LINEAR:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

static VkFilter convert_texture_filter(kinc_g5_texture_filter_t filter) {
	switch (filter) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		return VK_FILTER_NEAREST;
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		return VK_FILTER_LINEAR;
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return VK_FILTER_LINEAR; // ?
	}
}

void kinc_g5_sampler_create(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_descriptor_t *descriptor) {
	VkSamplerCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;

	info.addressModeU = convert_addressing(descriptor->u_addressing);
	info.addressModeV = convert_addressing(descriptor->v_addressing);
	info.addressModeW = convert_addressing(descriptor->w_addressing);

	info.mipmapMode = convert_mipmap_mode(descriptor->mipmap_filter);

	info.magFilter = convert_texture_filter(descriptor->magnification_filter);
	info.minFilter = convert_texture_filter(descriptor->minification_filter);

	info.compareEnable = descriptor->is_comparison;
	info.compareOp = convert_compare_mode(descriptor->compare_mode);

	info.anisotropyEnable =
	    (descriptor->magnification_filter == KINC_G5_TEXTURE_FILTER_ANISOTROPIC || descriptor->minification_filter == KINC_G5_TEXTURE_FILTER_ANISOTROPIC);
	info.maxAnisotropy = descriptor->max_anisotropy;

	info.maxLod = descriptor->lod_max_clamp;
	info.minLod = descriptor->lod_min_clamp;

	vkCreateSampler(vk_ctx.device, &info, NULL, &sampler->impl.sampler);
}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {
	vkDestroySampler(vk_ctx.device, sampler->impl.sampler, NULL);
}