#include "pch.h"

#include "Vulkan.h"

#include <Kore/Graphics5/ConstantBuffer.h>

#include <assert.h>
#include <memory.h>

using namespace Kore;

extern VkDevice device;
bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

VkBuffer* Kore::Vulkan::vertexUniformBuffer = nullptr;
VkBuffer* Kore::Vulkan::fragmentUniformBuffer = nullptr;

namespace {
	void createUniformBuffer(VkBuffer& buf, VkMemoryAllocateInfo& mem_alloc, VkDeviceMemory& mem, VkDescriptorBufferInfo& buffer_info) {
		VkBufferCreateInfo buf_info;
		memset(&buf_info, 0, sizeof(buf_info));
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buf_info.size = sizeof(float) * 256;
		VkResult err = vkCreateBuffer(device, &buf_info, NULL, &buf);
		assert(!err);

		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(device, buf, &mem_reqs);

		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = mem_reqs.size;
		mem_alloc.memoryTypeIndex = 0;

		bool pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
		assert(pass);

		err = vkAllocateMemory(device, &mem_alloc, NULL, &mem);
		assert(!err);

		err = vkBindBufferMemory(device, buf, mem, 0);
		assert(!err);

		buffer_info.buffer = buf;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(float) * 256;
	}
}

Graphics5::ConstantBuffer::ConstantBuffer(int size) {
	mySize = size;
	data = nullptr;

	createUniformBuffer(buf, mem_alloc, mem, buffer_info);

	uint8_t* data;
	VkResult err = vkMapMemory(device, mem, 0, mem_alloc.allocationSize, 0, (void**)&data);
	assert(!err);
	memcpy(data, &data, sizeof(data));
	vkUnmapMemory(device, mem);

	// buffer hack
	if (Vulkan::vertexUniformBuffer == nullptr) {
		Vulkan::vertexUniformBuffer = &buf;
	}
	else if (Vulkan::fragmentUniformBuffer == nullptr) {
		Vulkan::fragmentUniformBuffer = &buf;
	}
}

Graphics5::ConstantBuffer::~ConstantBuffer() {}

void Graphics5::ConstantBuffer::lock() {
	lock(0, size());
}

void Graphics5::ConstantBuffer::lock(int start, int count) {}

void Graphics5::ConstantBuffer::unlock() {

	data = nullptr;
}

int Graphics5::ConstantBuffer::size() {
	return mySize;
}

/*void Graphics5::ConstantBuffer::setBool(int offset, bool value) {
    if (offset >= 0) {
        int* data = (int*)&((u8*)data)[offset];
        data[0] = value;
    }
}

void Graphics5::ConstantBuffer::setInt(int offset, int value) {
    if (offset  >= 0) {
        int* data = (int*)&((u8*)data)[offset];
        data[0] = value;
    }
}

void Graphics5::ConstantBuffer::setFloat(int offset, float value) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        data[0] = value;
    }
}

void Graphics5::ConstantBuffer::setFloat2(int offset, float value1, float value2) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        data[0] = value1;
        data[1] = value2;
    }
}

void Graphics5::ConstantBuffer::setFloat3(int offset, float value1, float value2, float value3) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        data[0] = value1;
        data[1] = value2;
        data[2] = value3;
    }
}

void Graphics5::ConstantBuffer::setFloat4(int offset, float value1, float value2, float value3, float value4) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        data[0] = value1;
        data[1] = value2;
        data[2] = value3;
        data[3] = value4;
    }
}

void Graphics5::ConstantBuffer::setFloats(int offset, float* values, int count) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        for (int i = 0; i < count; ++i) {
            data[i] = values[i];
        }
    }
}

void Graphics5::ConstantBuffer::setMatrix(int offset, const mat4& value) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        for (int i = 0; i < 16; ++i) {
            data[i] = value.data[i];
        }
    }
}

void Graphics5::ConstantBuffer::setMatrix(int offset, const mat3& value) {
    if (offset >= 0) {
        float* data = (float*)&((u8*)data)[offset];
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                data[y * 4 + x] = value.data[y * 3 + x];
            }
        }
    }
}*/
