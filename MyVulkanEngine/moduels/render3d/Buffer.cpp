#include "Buffer.h"

namespace MVE
{
Buffer::Buffer(Device& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
			   VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment):
	device(device),
	instanceCount {instanceCount}, instanceSize {instanceSize}, usageFlags(usageFlags),
	memoryPropertyFlags(memoryPropertyFlags)
{
	alignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
	bufferSize	  = alignmentSize * instanceCount;
	device.CreateBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
}

Buffer::~Buffer()
{
	Unmap();
	vkDestroyBuffer(device.VulkanDevice(), buffer, nullptr);
	vkFreeMemory(device.VulkanDevice(), memory, nullptr);
}

VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
	MVE_ASSERT(buffer && memory, "Can't map uninitialized buffer.");
	return vkMapMemory(device.VulkanDevice(), memory, offset, size, 0, &mapped);
}

void Buffer::Unmap()
{
	if (mapped) {
		vkUnmapMemory(device.VulkanDevice(), memory);
		mapped = nullptr;
	}
}

void Buffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
	MVE_ASSERT(mapped, "cannot write to unmapped buffer");

	if (size == VK_WHOLE_SIZE) {
		memcpy(mapped, data, bufferSize);
	} else {
		char* memOffset = (char*)mapped;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}

VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType				= VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory				= memory;
	mappedRange.offset				= offset;
	mappedRange.size				= size;
	return vkFlushMappedMemoryRanges(device.VulkanDevice(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
	return VkDescriptorBufferInfo {
		buffer,
		offset,
		size,
	};
}

VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType				= VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory				= memory;
	mappedRange.offset				= offset;
	mappedRange.size				= size;
	return vkInvalidateMappedMemoryRanges(device.VulkanDevice(), 1, &mappedRange);
}

void Buffer::WriteToIndex(void* data, int index)
{
	WriteToBuffer(data, instanceSize, index * alignmentSize);
}

VkResult Buffer::FlushIndex(int index)
{
	return Flush(alignmentSize, index * alignmentSize);
}

VkDescriptorBufferInfo Buffer::DescriptorInfoForIndex(int index)
{
	return DescriptorInfo(alignmentSize, index * alignmentSize);
}

VkResult Buffer::InvalidateIndex(int index)
{
	return Invalidate(alignmentSize, index * alignmentSize);
}

VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
{
	if (minOffsetAlignment <= 0 || instanceSize % minOffsetAlignment == 0)
		return instanceSize;
	return (instanceSize / minOffsetAlignment + 1) * minOffsetAlignment;
}

} // namespace MVE