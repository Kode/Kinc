#include "vulkan.h"
#include <kinc/graphics6/bindgroup.h>

#include <malloc.h>

static VkDescriptorType convert_binding_type(kinc_g6_binding_type_t binding) {
	switch (binding) {
	case KINC_G6_BINDING_TYPE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case KINC_G6_BINDING_TYPE_TEXTURE:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case KINC_G6_BINDING_TYPE_STORAGE_TEXTURE:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case KINC_G6_BINDING_TYPE_UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case KINC_G6_BINDING_TYPE_STORAGE_BUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case KINC_G6_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case KINC_G6_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	}
}

static VkShaderStageFlags convert_stage_flags(kinc_g6_shader_stage_flags_t flags) {
	VkShaderStageFlags f = 0;
	if (flags & KINC_G6_SHADER_STAGE_VERTEX) f |= VK_SHADER_STAGE_VERTEX_BIT;
	if (flags & KINC_G6_SHADER_STAGE_FRAGMENT) f |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (flags & KINC_G6_SHADER_STAGE_COMPUTE) f |= VK_SHADER_STAGE_COMPUTE_BIT;
	return f;
}

void kinc_g6_bind_group_layout_init(kinc_g6_bind_group_layout_t *layout, const kinc_g6_bind_group_layout_descriptor_t *descriptor) {
	VkDescriptorSetLayoutCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.bindingCount = descriptor->entry_count;
	VkDescriptorSetLayoutBinding *bindings = alloca(sizeof(*bindings) * descriptor->entry_count);
	layout->impl.types = malloc(sizeof(*layout->impl.types) * descriptor->entry_count);
	for (int i = 0; i < descriptor->entry_count; i++) {
		VkDescriptorSetLayoutBinding *b = &bindings[i];
		kinc_g6_bind_group_layout_entry_t *entry = &descriptor->entries[i];
		b->binding = layout->impl.types[i].binding = entry->binding;
		b->descriptorCount = 1;
		b->descriptorType = layout->impl.types[i].type = convert_binding_type(entry->type);
		b->pImmutableSamplers = NULL;
		b->stageFlags = convert_stage_flags(entry->visibility);
	}
	create_info.pBindings = bindings;
	layout->impl.count = descriptor->entry_count;
	vkCreateDescriptorSetLayout(context.device, &create_info, NULL, layout->impl.layout);
}

void kinc_g6_bind_group_layout_destroy(kinc_g6_bind_group_layout_t *layout) {
	vkDestroyDescriptorSetLayout(context.device, layout->impl.layout, NULL);
}

void kinc_g6_bind_group_init(kinc_g6_bind_group_t *group, const kinc_g6_bind_group_descriptor_t *descriptor) {
	VkDescriptorSetAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.descriptorPool = NULL;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &descriptor->layout->impl.layout;
	vkAllocateDescriptorSets(context.device, &alloc_info, &group->impl.set);

	VkWriteDescriptorSet *write_sets = alloca(sizeof(*write_sets) * descriptor->entry_count);
	for (int i = 0; i < descriptor->entry_count; i++) {
		write_sets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_sets[i].pNext = NULL;
		write_sets[i].dstSet = group->impl.set;
		write_sets[i].dstBinding = descriptor->entries[i].binding;
		write_sets[i].descriptorCount = 1;
		write_sets[i].descriptorType = convert_binding_type(descriptor->layout->impl.types[i].type);
		switch (descriptor->layout->impl.types[i].type) {
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			write_sets[i].pImageInfo = &(VkDescriptorImageInfo){.sampler = descriptor->entries[i].sampler};
			break;
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			write_sets[i].pImageInfo =
			    &(VkDescriptorImageInfo){.imageView = descriptor->entries[i].texture->impl.view, .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			write_sets[i].pImageInfo = &(VkDescriptorImageInfo){.imageView = descriptor->entries[i].texture->impl.view, .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			write_sets[i].pBufferInfo = &(VkDescriptorBufferInfo){
			    .buffer = descriptor->entries[i].buffer.buffer, .offset = descriptor->entries[i].buffer.offset, .range = descriptor->entries[i].buffer.size};
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			write_sets[i].pBufferInfo = &(VkDescriptorBufferInfo){
			    .buffer = descriptor->entries[i].buffer.buffer, .offset = descriptor->entries[i].buffer.offset, .range = descriptor->entries[i].buffer.size};
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			write_sets[i].pBufferInfo = &(VkDescriptorBufferInfo){
			    .buffer = descriptor->entries[i].buffer.buffer, .offset = descriptor->entries[i].buffer.offset, .range = descriptor->entries[i].buffer.size};
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			write_sets[i].pBufferInfo = &(VkDescriptorBufferInfo){
			    .buffer = descriptor->entries[i].buffer.buffer, .offset = descriptor->entries[i].buffer.offset, .range = descriptor->entries[i].buffer.size};
			break;
		}
	}
	vkUpdateDescriptorSets(context.device,descriptor->entries,write_sets,0,NULL);
}

void kinc_g6_bind_group_destroy(kinc_g6_bind_group_t *group) {
	vkFreeDescriptorSets(context.device, NULL, 1, &group->impl.set);
}