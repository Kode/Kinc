#include "pch.h"

#include "RayTraceImpl.h"

#ifdef KORE_VKRT

#include "Vulkan.h"
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/raytrace.h>

extern VkDevice device;
extern VkQueue queue;
extern VkCommandPool cmd_pool;
extern VkPhysicalDevice gpu;
extern VkRenderPassBeginInfo currentRenderPassBeginInfo;
extern VkFramebuffer *framebuffers;
extern uint32_t current_buffer;
bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

const int INDEX_RAYGEN = 0;
const int INDEX_CLOSEST_HIT = 1;
const int INDEX_MISS = 2;
const char* raygen_shader_name = "raygeneration";
const char* closesthit_shader_name = "closesthit";
const char* miss_shader_name = "miss";

VkDescriptorPool descriptor_pool;
kinc_raytrace_acceleration_structure_t* accel;
kinc_raytrace_pipeline_t* pipeline;
kinc_g5_texture_t* output = nullptr;

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list *command_list, void *ray_shader, int ray_shader_size, kinc_g5_constant_buffer_t *constant_buffer) {
	pipeline->_constant_buffer = constant_buffer;

	{
		VkDescriptorSetLayoutBinding acceleration_structure_layout_binding{};
		acceleration_structure_layout_binding.binding = 0;
		acceleration_structure_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		acceleration_structure_layout_binding.descriptorCount = 1;
		acceleration_structure_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding result_image_layout_binding{};
		result_image_layout_binding.binding = 1;
		result_image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		result_image_layout_binding.descriptorCount = 1;
		result_image_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding uniform_buffer_binding{};
		uniform_buffer_binding.binding = 2;
		uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_buffer_binding.descriptorCount = 1;
		uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding bindings[3] = { acceleration_structure_layout_binding, result_image_layout_binding, uniform_buffer_binding };

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = NULL;
		layout_info.bindingCount = 3;
		layout_info.pBindings = &bindings[0];
		vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &pipeline->impl.descriptor_set_layout);

		VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext = NULL;
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &pipeline->impl.descriptor_set_layout;

		vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline->impl.pipeline_layout);

		VkShaderModuleCreateInfo module_create_info{};
		memset(&module_create_info, 0, sizeof(VkShaderModuleCreateInfo));
		module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_create_info.codeSize = ray_shader_size;
		module_create_info.pCode = (const uint32_t *)ray_shader;
		module_create_info.pNext = nullptr;
		module_create_info.flags = 0;
		VkShaderModule shader_module;
		vkCreateShaderModule(device, &module_create_info, NULL, &shader_module);

		VkPipelineShaderStageCreateInfo shader_stages[3];
		shader_stages[INDEX_RAYGEN].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[INDEX_RAYGEN].pNext = NULL;
		shader_stages[INDEX_RAYGEN].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		shader_stages[INDEX_RAYGEN].module = shader_module;
		shader_stages[INDEX_RAYGEN].pName = raygen_shader_name;
		shader_stages[INDEX_RAYGEN].flags = 0;
		shader_stages[INDEX_RAYGEN].pSpecializationInfo = NULL;

		shader_stages[INDEX_CLOSEST_HIT].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[INDEX_CLOSEST_HIT].pNext = NULL;
		shader_stages[INDEX_CLOSEST_HIT].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		shader_stages[INDEX_CLOSEST_HIT].module = shader_module;
		shader_stages[INDEX_CLOSEST_HIT].pName = closesthit_shader_name;
		shader_stages[INDEX_CLOSEST_HIT].flags = 0;
		shader_stages[INDEX_CLOSEST_HIT].pSpecializationInfo = NULL;

		shader_stages[INDEX_MISS].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[INDEX_MISS].pNext = NULL;
		shader_stages[INDEX_MISS].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		shader_stages[INDEX_MISS].module = shader_module;
		shader_stages[INDEX_MISS].pName = miss_shader_name;
		shader_stages[INDEX_MISS].flags = 0;
		shader_stages[INDEX_MISS].pSpecializationInfo = NULL;

		VkRayTracingShaderGroupCreateInfoKHR groups[3];
		groups[INDEX_RAYGEN].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groups[INDEX_RAYGEN].pNext = NULL;
		groups[INDEX_RAYGEN].generalShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].closestHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].anyHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].intersectionShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		groups[INDEX_RAYGEN].generalShader = INDEX_RAYGEN;

		groups[INDEX_CLOSEST_HIT].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groups[INDEX_CLOSEST_HIT].pNext = NULL;
		groups[INDEX_CLOSEST_HIT].generalShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].closestHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].anyHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].intersectionShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		groups[INDEX_CLOSEST_HIT].closestHitShader = INDEX_CLOSEST_HIT;

		groups[INDEX_MISS].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groups[INDEX_MISS].pNext = NULL;
		groups[INDEX_MISS].generalShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].closestHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].anyHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].intersectionShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		groups[INDEX_MISS].generalShader = INDEX_MISS;

		VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info = {};
		raytracing_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		raytracing_pipeline_create_info.pNext = NULL;
		raytracing_pipeline_create_info.flags = 0;
		raytracing_pipeline_create_info.stageCount = 3;
		raytracing_pipeline_create_info.pStages = &shader_stages[0];
		raytracing_pipeline_create_info.groupCount = 3;
		raytracing_pipeline_create_info.pGroups = &groups[0];
		raytracing_pipeline_create_info.maxRecursionDepth = 1;
		raytracing_pipeline_create_info.layout = pipeline->impl.pipeline_layout;
		raytracing_pipeline_create_info.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
		raytracing_pipeline_create_info.libraries.libraryCount = 0;
		raytracing_pipeline_create_info.libraries.pNext = NULL;
		auto vkCreateRayTracingPipelinesKHR = PFN_vkCreateRayTracingPipelinesKHR(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
		vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, nullptr, &pipeline->impl.pipeline);
	}

	{
		VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_properties;
		ray_tracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR;
		ray_tracing_properties.pNext = nullptr;
		VkPhysicalDeviceProperties2 device_properties{};
		device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		device_properties.pNext = &ray_tracing_properties;
		vkGetPhysicalDeviceProperties2(gpu, &device_properties);

		uint32_t shader_binding_table_size = ray_tracing_properties.shaderGroupHandleSize * 3;

		VkBufferCreateInfo buf_info = {};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.size = shader_binding_table_size;
		buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR;
		buf_info.flags = 0;

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkMemoryRequirements mem_reqs = {};
		vkCreateBuffer(device, &buf_info, NULL, &pipeline->impl.shader_binding_table);
		vkGetBufferMemoryRequirements(device, pipeline->impl.shader_binding_table, &mem_reqs);

		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		VkDeviceMemory mem;
		vkAllocateMemory(device, &mem_alloc, NULL, &mem);

		vkBindBufferMemory(device, pipeline->impl.shader_binding_table, mem, 0);
		void* data;
		vkMapMemory(device, mem, 0, shader_binding_table_size, 0, (void **)&data);
		auto vkGetRayTracingShaderGroupHandlesKHR = PFN_vkGetRayTracingShaderGroupHandlesKHR(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
		vkGetRayTracingShaderGroupHandlesKHR(device, pipeline->impl.pipeline, 0, 3, shader_binding_table_size, data);
		vkUnmapMemory(device, mem);
	}

	{
		VkDescriptorPoolSize type_counts[3];
		memset(type_counts, 0, sizeof(type_counts));

		type_counts[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		type_counts[0].descriptorCount = 1;

		type_counts[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		type_counts[1].descriptorCount = 1;

		type_counts[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		type_counts[2].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.pNext = NULL;
		descriptor_pool_create_info.maxSets = 1024;
		descriptor_pool_create_info.poolSizeCount = 3;
		descriptor_pool_create_info.pPoolSizes = type_counts;

		vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &descriptor_pool);

		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.pNext = NULL;
		alloc_info.descriptorPool = descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &pipeline->impl.descriptor_set_layout;
		vkAllocateDescriptorSets(device, &alloc_info, &pipeline->impl.descriptor_set);
	}
}

void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline) {
	vkDestroyPipeline(device, pipeline->impl.pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipeline->impl.pipeline_layout, nullptr);
	vkDestroyDescriptorSetLayout(device, pipeline->impl.descriptor_set_layout, nullptr);
}

uint64_t get_buffer_device_address(VkBuffer buffer) {
	VkBufferDeviceAddressInfoKHR buffer_device_address_info = {};
	buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	buffer_device_address_info.buffer = buffer;
	auto vkGetBufferDeviceAddressKHR = PFN_vkGetBufferDeviceAddressKHR(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
	return vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list, kinc_g5_vertex_buffer_t *vb, kinc_g5_index_buffer_t *ib) {

	auto vkGetAccelerationStructureMemoryRequirementsKHR = PFN_vkGetAccelerationStructureMemoryRequirementsKHR(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureMemoryRequirementsKHR"));
	auto vkCreateAccelerationStructureKHR = PFN_vkCreateAccelerationStructureKHR(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
	auto vkBindAccelerationStructureMemoryKHR = PFN_vkBindAccelerationStructureMemoryKHR(vkGetDeviceProcAddr(device, "vkBindAccelerationStructureMemoryKHR"));
	auto vkGetAccelerationStructureDeviceAddressKHR = PFN_vkGetAccelerationStructureDeviceAddressKHR(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	auto vkGetBufferDeviceAddressKHR = PFN_vkGetBufferDeviceAddressKHR(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));

	{
		VkDeviceOrHostAddressConstKHR vertex_data_device_address{};
		VkDeviceOrHostAddressConstKHR index_data_device_address{};

		vertex_data_device_address.deviceAddress = get_buffer_device_address(vb->impl.vertices.buf);
		index_data_device_address.deviceAddress = get_buffer_device_address(ib->impl.buf);

		VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info{};
		acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		acceleration_create_geometry_info.maxPrimitiveCount = 1;
		acceleration_create_geometry_info.indexType = VK_INDEX_TYPE_UINT32;
		acceleration_create_geometry_info.maxVertexCount = vb->impl.myCount;
		acceleration_create_geometry_info.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		acceleration_create_geometry_info.allowsTransforms = VK_FALSE;

		VkAccelerationStructureCreateInfoKHR acceleration_create_info{};
		acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		acceleration_create_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		acceleration_create_info.maxGeometryCount = 1;
		acceleration_create_info.pGeometryInfos = &acceleration_create_geometry_info;
		vkCreateAccelerationStructureKHR(device, &acceleration_create_info, nullptr, &accel->impl.bottom_level_acceleration_structure);

		// Bind object memory to the bottom level acceleration structure
		VkMemoryRequirements2 memory_requirements_2{};
		memory_requirements_2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

		VkAccelerationStructureMemoryRequirementsInfoKHR acceleration_memory_requirements{};
		acceleration_memory_requirements.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		acceleration_memory_requirements.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
		acceleration_memory_requirements.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		acceleration_memory_requirements.accelerationStructure = accel->impl.bottom_level_acceleration_structure;
		vkGetAccelerationStructureMemoryRequirementsKHR(device, &acceleration_memory_requirements, &memory_requirements_2);

		VkMemoryRequirements memory_requirements = memory_requirements_2.memoryRequirements;

		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);

		VkDeviceMemory memory = VK_NULL_HANDLE;
		vkAllocateMemory(device, &memory_allocate_info, nullptr, &memory);

		VkBindAccelerationStructureMemoryInfoKHR bind_acceleration_memory_info{};
		bind_acceleration_memory_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
		bind_acceleration_memory_info.accelerationStructure = accel->impl.bottom_level_acceleration_structure;
		bind_acceleration_memory_info.memory = memory;
		vkBindAccelerationStructureMemoryKHR(device, 1, &bind_acceleration_memory_info);

		VkAccelerationStructureGeometryKHR acceleration_geometry{};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		acceleration_geometry.geometry.triangles.vertexData.deviceAddress = vertex_data_device_address.deviceAddress;
		acceleration_geometry.geometry.triangles.vertexStride = vb->impl.myStride;
		acceleration_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		acceleration_geometry.geometry.triangles.indexData.deviceAddress = index_data_device_address.deviceAddress;

		VkBuffer scratch_buffer = VK_NULL_HANDLE;
		VkDeviceMemory scratch_memory = VK_NULL_HANDLE;

		acceleration_memory_requirements.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		acceleration_memory_requirements.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
		acceleration_memory_requirements.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		acceleration_memory_requirements.accelerationStructure = accel->impl.bottom_level_acceleration_structure;
		vkGetAccelerationStructureMemoryRequirementsKHR(device, &acceleration_memory_requirements, &memory_requirements_2);

		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = memory_requirements_2.memoryRequirements.size;
		buffer_create_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(device, &buffer_create_info, nullptr, &scratch_buffer);

		vkGetBufferMemoryRequirements(device, scratch_buffer, &memory_requirements);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info{};
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.pNext = &memory_allocate_flags_info;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		vkAllocateMemory(device, &memory_allocate_info, nullptr, &scratch_memory);
		vkBindBufferMemory(device, scratch_buffer, scratch_memory, 0);

		VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
		buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		buffer_device_address_info.buffer = scratch_buffer;
		uint64_t scratch_buffer_device_address = vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);

		VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info{};
		acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		acceleration_build_geometry_info.update = VK_FALSE;
		acceleration_build_geometry_info.dstAccelerationStructure = accel->impl.bottom_level_acceleration_structure;
		acceleration_build_geometry_info.geometryArrayOfPointers = VK_FALSE;
		acceleration_build_geometry_info.geometryCount = 1;

		VkAccelerationStructureGeometryKHR * const acceleration_geometries[1] = { &acceleration_geometry };
		acceleration_build_geometry_info.ppGeometries = &acceleration_geometries[0];
		acceleration_build_geometry_info.scratchData.deviceAddress = scratch_buffer_device_address;

		VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info{};
		acceleration_build_offset_info.primitiveCount = 1;
		acceleration_build_offset_info.primitiveOffset = 0x0;
		acceleration_build_offset_info.firstVertex = 0;
		acceleration_build_offset_info.transformOffset = 0x0;

		VkAccelerationStructureBuildOffsetInfoKHR *acceleration_build_offsets[1] = {&acceleration_build_offset_info};

		{
			VkCommandBufferAllocateInfo cmd_buf_allocate_info{};
			cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_buf_allocate_info.commandPool = cmd_pool;
			cmd_buf_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_buf_allocate_info.commandBufferCount = 1;

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(device, &cmd_buf_allocate_info, &command_buffer);

			VkCommandBufferBeginInfo command_buffer_info{};
			command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			vkBeginCommandBuffer(command_buffer, &command_buffer_info);

			auto vkCmdBuildAccelerationStructureKHR = PFN_vkCmdBuildAccelerationStructureKHR(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructureKHR"));
			vkCmdBuildAccelerationStructureKHR(command_buffer, 1, &acceleration_build_geometry_info, &acceleration_build_offsets[0]);

			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submit_info{};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer;

			VkFenceCreateInfo fence_info{};
			fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_info.flags = 0;

			VkFence fence;
			vkCreateFence(device, &fence_info, nullptr, &fence);

			VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
			vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
			vkDestroyFence(device, fence, nullptr);
			vkFreeCommandBuffers(device, cmd_pool, 1, &command_buffer);
		}

		VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info{};
		acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		acceleration_device_address_info.accelerationStructure = accel->impl.bottom_level_acceleration_structure;

		accel->impl.bottom_level_acceleration_structure_handle = vkGetAccelerationStructureDeviceAddressKHR(device, &acceleration_device_address_info);

		vkFreeMemory(device, scratch_memory, nullptr);
		vkDestroyBuffer(device, scratch_buffer, nullptr);
	}

	{
		VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info{};
		acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		acceleration_create_geometry_info.maxPrimitiveCount = 1;
		acceleration_create_geometry_info.allowsTransforms = VK_FALSE;

		VkAccelerationStructureCreateInfoKHR acceleration_create_info{};
		acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		acceleration_create_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		acceleration_create_info.maxGeometryCount = 1;
		acceleration_create_info.pGeometryInfos = &acceleration_create_geometry_info;
		vkCreateAccelerationStructureKHR(device, &acceleration_create_info, nullptr, &accel->impl.top_level_acceleration_structure);

		// Bind object memory to the top level acceleration structure
		VkMemoryRequirements2 memory_requirements_2{};
		memory_requirements_2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

		VkAccelerationStructureMemoryRequirementsInfoKHR acceleration_memory_requirements{};
		acceleration_memory_requirements.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		acceleration_memory_requirements.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
		acceleration_memory_requirements.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		acceleration_memory_requirements.accelerationStructure = accel->impl.top_level_acceleration_structure;
		vkGetAccelerationStructureMemoryRequirementsKHR(device, &acceleration_memory_requirements, &memory_requirements_2);

		VkMemoryRequirements memory_requirements = memory_requirements_2.memoryRequirements;

		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);

		VkDeviceMemory memory = VK_NULL_HANDLE;
		vkAllocateMemory(device, &memory_allocate_info, nullptr, &memory);

		VkBindAccelerationStructureMemoryInfoKHR bind_acceleration_memory_info{};
		bind_acceleration_memory_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
		bind_acceleration_memory_info.accelerationStructure = accel->impl.top_level_acceleration_structure;
		bind_acceleration_memory_info.memory = memory;
		vkBindAccelerationStructureMemoryKHR(device, 1, &bind_acceleration_memory_info);

		VkTransformMatrixKHR transform_matrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f};

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transform_matrix;
		instance.instanceCustomIndex = 0;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = accel->impl.bottom_level_acceleration_structure_handle;

		VkBuffer instances_buffer;

		VkBufferCreateInfo buf_info = {};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.size = sizeof(instance);
		buf_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.flags = 0;

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkMemoryRequirements mem_reqs = {};

		vkCreateBuffer(device, &buf_info, NULL, &instances_buffer);

		vkGetBufferMemoryRequirements(device, instances_buffer, &mem_reqs);

		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {};
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		mem_alloc.pNext = &memory_allocate_flags_info;

		VkDeviceMemory mem;
		vkAllocateMemory(device, &mem_alloc, NULL, &mem);

		vkBindBufferMemory(device, instances_buffer, mem, 0);
		void* data;
		vkMapMemory(device, mem, 0, sizeof(VkAccelerationStructureInstanceKHR), 0, (void **)&data);
		memcpy(data, &instance, sizeof(VkAccelerationStructureInstanceKHR));
		vkUnmapMemory(device, mem);

		VkDeviceOrHostAddressConstKHR instance_data_device_address{};
		instance_data_device_address.deviceAddress = get_buffer_device_address(instances_buffer);

		VkAccelerationStructureGeometryKHR acceleration_geometry{};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		acceleration_geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		acceleration_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
		acceleration_geometry.geometry.instances.data.deviceAddress = instance_data_device_address.deviceAddress;

		VkAccelerationStructureGeometryKHR * const acceleration_geometries[1] = { &acceleration_geometry };

		VkBuffer scratch_buffer = VK_NULL_HANDLE;
		VkDeviceMemory scratch_memory = VK_NULL_HANDLE;

		acceleration_memory_requirements.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		acceleration_memory_requirements.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
		acceleration_memory_requirements.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		acceleration_memory_requirements.accelerationStructure = accel->impl.bottom_level_acceleration_structure;
		vkGetAccelerationStructureMemoryRequirementsKHR(device, &acceleration_memory_requirements, &memory_requirements_2);

		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = memory_requirements_2.memoryRequirements.size;
		buffer_create_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(device, &buffer_create_info, nullptr, &scratch_buffer);

		vkGetBufferMemoryRequirements(device, scratch_buffer, &memory_requirements);

		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.pNext = &memory_allocate_flags_info;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		vkAllocateMemory(device, &memory_allocate_info, nullptr, &scratch_memory);
		vkBindBufferMemory(device, scratch_buffer, scratch_memory, 0);

		VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
		buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		buffer_device_address_info.buffer = scratch_buffer;
		uint64_t scratch_buffer_device_address = vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);

		VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info{};
		acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		acceleration_build_geometry_info.update = VK_FALSE;
		acceleration_build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
		acceleration_build_geometry_info.dstAccelerationStructure = accel->impl.top_level_acceleration_structure;
		acceleration_build_geometry_info.geometryArrayOfPointers = VK_FALSE;
		acceleration_build_geometry_info.geometryCount = 1;
		acceleration_build_geometry_info.ppGeometries = &acceleration_geometries[0];
		acceleration_build_geometry_info.scratchData.deviceAddress = scratch_buffer_device_address;

		VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info{};
		acceleration_build_offset_info.primitiveCount = 1;
		acceleration_build_offset_info.primitiveOffset = 0x0;
		acceleration_build_offset_info.firstVertex = 0;
		acceleration_build_offset_info.transformOffset = 0x0;

		VkAccelerationStructureBuildOffsetInfoKHR *acceleration_build_offsets[1] = {&acceleration_build_offset_info};

		{
			VkCommandBufferAllocateInfo cmd_buf_allocate_info{};
			cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_buf_allocate_info.commandPool = cmd_pool;
			cmd_buf_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_buf_allocate_info.commandBufferCount = 1;

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(device, &cmd_buf_allocate_info, &command_buffer);

			VkCommandBufferBeginInfo command_buffer_info{};
			command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			vkBeginCommandBuffer(command_buffer, &command_buffer_info);

			auto vkCmdBuildAccelerationStructureKHR = PFN_vkCmdBuildAccelerationStructureKHR(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructureKHR"));
			vkCmdBuildAccelerationStructureKHR(command_buffer, 1, &acceleration_build_geometry_info, &acceleration_build_offsets[0]);

			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submit_info{};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer;

			VkFenceCreateInfo fence_info{};
			fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_info.flags = 0;

			VkFence fence;
			vkCreateFence(device, &fence_info, nullptr, &fence);

			VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
			vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
			vkDestroyFence(device, fence, nullptr);

			vkFreeCommandBuffers(device, cmd_pool, 1, &command_buffer);
		}

		VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info{};
		acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		acceleration_device_address_info.accelerationStructure = accel->impl.top_level_acceleration_structure;

		accel->impl.top_level_acceleration_structure_handle = vkGetAccelerationStructureDeviceAddressKHR(device, &acceleration_device_address_info);

		vkFreeMemory(device, scratch_memory, nullptr);
		vkDestroyBuffer(device, scratch_buffer, nullptr);
	}
}

void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel) {
	auto vkDestroyAccelerationStructureKHR = PFN_vkDestroyAccelerationStructureKHR(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
	vkDestroyAccelerationStructureKHR(device, accel->impl.bottom_level_acceleration_structure, nullptr);
	vkDestroyAccelerationStructureKHR(device, accel->impl.top_level_acceleration_structure, nullptr);
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_g5_texture_t *_output) {
	output = _output;
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration_structure_info{};
	descriptor_acceleration_structure_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptor_acceleration_structure_info.accelerationStructureCount = 1;
	descriptor_acceleration_structure_info.pAccelerationStructures = &accel->impl.top_level_acceleration_structure;

	VkWriteDescriptorSet acceleration_structure_write{};
	acceleration_structure_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	acceleration_structure_write.pNext = &descriptor_acceleration_structure_info;
	acceleration_structure_write.dstSet = pipeline->impl.descriptor_set;
	acceleration_structure_write.dstBinding = 0;
	acceleration_structure_write.descriptorCount = 1;
	acceleration_structure_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	VkDescriptorImageInfo image_descriptor{};
	image_descriptor.imageView = output->impl.texture.view;
	image_descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkDescriptorBufferInfo buffer_descriptor{};
	buffer_descriptor.buffer = pipeline->_constant_buffer->impl.buf;
	buffer_descriptor.range = VK_WHOLE_SIZE;
	buffer_descriptor.offset = 0;

	VkWriteDescriptorSet result_image_write{};
	result_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	result_image_write.pNext = nullptr;
	result_image_write.dstSet = pipeline->impl.descriptor_set;
	result_image_write.dstBinding = 1;
	result_image_write.descriptorCount = 1;
	result_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	result_image_write.pImageInfo = &image_descriptor;

	VkWriteDescriptorSet uniform_buffer_write{};
	uniform_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniform_buffer_write.pNext = nullptr;
	uniform_buffer_write.dstSet = pipeline->impl.descriptor_set;
	uniform_buffer_write.dstBinding = 2;
	uniform_buffer_write.descriptorCount = 1;
	uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_write.pBufferInfo = &buffer_descriptor;

	VkWriteDescriptorSet write_descriptor_sets[3] = { acceleration_structure_write, result_image_write, uniform_buffer_write };
	vkUpdateDescriptorSets(device, 3, write_descriptor_sets, 0, VK_NULL_HANDLE);

	VkPhysicalDeviceRayTracingPropertiesKHR ray_tracing_properties;
	ray_tracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR;
	ray_tracing_properties.pNext = nullptr;
	VkPhysicalDeviceProperties2 device_properties{};
	device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	device_properties.pNext = &ray_tracing_properties;
	vkGetPhysicalDeviceProperties2(gpu, &device_properties);

	// Setup the strided buffer regions pointing to the shaders in our shader binding table
	VkStridedBufferRegionKHR raygen_shader_sbt_entry{};
	raygen_shader_sbt_entry.buffer = pipeline->impl.shader_binding_table;
	raygen_shader_sbt_entry.offset = (VkDeviceSize)(ray_tracing_properties.shaderGroupHandleSize * INDEX_RAYGEN);
	raygen_shader_sbt_entry.size = ray_tracing_properties.shaderGroupHandleSize;

	VkStridedBufferRegionKHR miss_shader_sbt_entry{};
	miss_shader_sbt_entry.buffer = pipeline->impl.shader_binding_table;
	miss_shader_sbt_entry.offset = (VkDeviceSize)(ray_tracing_properties.shaderGroupHandleSize * INDEX_MISS);
	miss_shader_sbt_entry.size = ray_tracing_properties.shaderGroupHandleSize;

	VkStridedBufferRegionKHR hit_shader_sbt_entry{};
	hit_shader_sbt_entry.buffer = pipeline->impl.shader_binding_table;
	hit_shader_sbt_entry.offset = (VkDeviceSize)(ray_tracing_properties.shaderGroupHandleSize * INDEX_CLOSEST_HIT);
	hit_shader_sbt_entry.size = ray_tracing_properties.shaderGroupHandleSize;

	VkStridedBufferRegionKHR callable_shader_sbt_entry{};

	vkCmdEndRenderPass(command_list->impl._buffer);

	// Dispatch the ray tracing commands
	vkCmdBindPipeline(command_list->impl._buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline);
	vkCmdBindDescriptorSets(command_list->impl._buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline_layout, 0, 1, &pipeline->impl.descriptor_set, 0, 0);

	auto vkCmdTraceRaysKHR = PFN_vkCmdTraceRaysKHR(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
	vkCmdTraceRaysKHR(command_list->impl._buffer, &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry, output->texWidth, output->texHeight, 1);

	vkCmdBeginRenderPass(command_list->impl._buffer, &currentRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void kinc_raytrace_copy(kinc_g5_command_list_t *command_list, kinc_g5_render_target_t* target, kinc_g5_texture_t *source) {

	vkCmdEndRenderPass(command_list->impl._buffer);

	VkImageCopy copy_region{};
	copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
	copy_region.srcOffset = {0, 0, 0};
	copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
	copy_region.dstOffset = {0, 0, 0};
	copy_region.extent = {(uint32_t)output->texWidth, (uint32_t)output->texHeight, 1};

	if (target->contextId < 0) {
		vkCmdCopyImage(command_list->impl._buffer, output->impl.texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   Kore::Vulkan::buffers[current_buffer].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
	}
	else {
		vkCmdCopyImage(command_list->impl._buffer, output->impl.texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   target->impl.sourceImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
	}

	vkCmdBeginRenderPass(command_list->impl._buffer, &currentRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

#endif // KORE_VKRT
