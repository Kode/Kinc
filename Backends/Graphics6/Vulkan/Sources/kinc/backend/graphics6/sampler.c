#include "vulkan.h"
#include <kinc/graphics6/sampler.h>

VkSamplerAddressMode convert_address_mode(kinc_g6_address_mode_t mode) {
	switch (mode) {
	case KINC_G6_ADDRESS_MODE_CLAMP_TO_EDGE:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case KINC_G6_ADDRESS_MODE_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case KINC_G6_ADDRESS_MODE_MIRROR_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	}
}

VkFilter convert_filter(kinc_g6_filter_mode_t filter) {
	switch (filter) {
	case KINC_G6_FILTER_MODE_NEAREST:
		return VK_FILTER_NEAREST;
	case KINC_G6_FILTER_MODE_LINEAR:
		return VK_FILTER_LINEAR;
	}
}

VkSamplerMipmapMode convert_filter_mip(kinc_g6_filter_mode_t filter) {
	switch (filter) {
	case KINC_G6_FILTER_MODE_NEAREST:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case KINC_G6_FILTER_MODE_LINEAR:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

VkCompareOp convert_compare(kinc_g6_compare_function_t compare);

KINC_FUNC void kinc_g6_sampler_init(kinc_g6_sampler_t *sampler, const kinc_g6_sampler_descriptor_t *descriptor) {
	VkSamplerCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.addressModeU = convert_address_mode(descriptor->address_mode_u);
	create_info.addressModeV = convert_address_mode(descriptor->address_mode_v);
	create_info.addressModeW = convert_address_mode(descriptor->address_mode_w);

	create_info.magFilter = convert_filter(descriptor->mag_filter);
	create_info.minFilter = convert_filter(descriptor->min_filter);
	create_info.mipmapMode = convert_filter_mip(descriptor->mipmap_filter);

	create_info.maxLod = descriptor->lod_max_clamp;
	create_info.minLod = descriptor->lod_min_clamp;

	create_info.compareEnable = descriptor->enable_compare;
	create_info.compareOp = convert_compare(descriptor->compare);

	create_info.anisotropyEnable = descriptor->enable_anisotropy;
	create_info.maxAnisotropy = descriptor->max_anisotropy;
	vkCreateSampler(context.device, &create_info, NULL, &sampler->impl.sampler);
}

KINC_FUNC void kinc_g6_sampler_destroy(kinc_g6_sampler_t *sampler) {
	vkDestroySampler(context.device, &sampler->impl.sampler, NULL);
}