#include "descriptorset_functions.h"
#include "descriptorset_structs.h"

#include <kope/vulkan/texture_functions.h>

#include <kope/util/align.h>

void kope_vulkan_descriptor_set_set_texture_view(kope_g5_device *device, kope_vulkan_descriptor_set *set, const kope_g5_texture_view *texture_view,
                                                 uint32_t index) {
	VkDescriptorImageInfo image_info = {
	    .imageView = texture_view->texture->vulkan.image_view,
	    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};

	VkWriteDescriptorSet write = {
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = set->descriptor_set,
	    .dstBinding = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	    .pImageInfo = &image_info,
	};

	vkUpdateDescriptorSets(device->vulkan.device, 1, &write, 0, NULL);
}

void kope_vulkan_descriptor_set_set_sampler(kope_g5_device *device, kope_vulkan_descriptor_set *set, kope_g5_sampler *sampler, uint32_t index) {
	VkDescriptorImageInfo image_info = {
	    .sampler = sampler->vulkan.sampler,
	    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};

	VkWriteDescriptorSet write = {
	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstSet = set->descriptor_set,
	    .dstBinding = index,
	    .descriptorCount = 1,
	    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
	    .pImageInfo = &image_info,
	};

	vkUpdateDescriptorSets(device->vulkan.device, 1, &write, 0, NULL);
}

void kope_vulkan_descriptor_set_prepare_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer) {}

void kope_vulkan_descriptor_set_prepare_texture(kope_g5_command_list *list, const kope_g5_texture_view *texture_view) {}
