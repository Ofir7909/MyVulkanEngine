#include "Descriptors.h"

namespace MVE
{
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(uint32_t binding,
																	   VkDescriptorType descriptorType,
																	   VkShaderStageFlags stageFlags, uint32_t count)
{
	MVE_ASSERT(bindings.count(binding) == 0, "Binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding {};
	layoutBinding.binding		  = binding;
	layoutBinding.descriptorType  = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags	  = stageFlags;
	bindings[binding]			  = layoutBinding;
	return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const
{
	return std::make_unique<DescriptorSetLayout>(device, bindings);
}

DescriptorSetLayout::DescriptorSetLayout(Device& device,
										 std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings):
	device {device},
	bindings {bindings}
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings {};
	for (auto kv : bindings) { setLayoutBindings.push_back(kv.second); }

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
	descriptorSetLayoutInfo.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings	 = setLayoutBindings.data();

	auto result =
		vkCreateDescriptorSetLayout(device.VulkanDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
	MVE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device.VulkanDevice(), descriptorSetLayout, nullptr);
}

DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
	poolSizes.push_back({descriptorType, count});
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
{
	poolFlags = flags;
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t count)
{
	maxSets = count;
	return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const
{
	return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
}

DescriptorPool::DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
							   const std::vector<VkDescriptorPoolSize>& poolSizes):
	device {device}
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo {};
	descriptorPoolInfo.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes	 = poolSizes.data();
	descriptorPoolInfo.maxSets		 = maxSets;
	descriptorPoolInfo.flags		 = poolFlags;

	auto result = vkCreateDescriptorPool(device.VulkanDevice(), &descriptorPoolInfo, nullptr, &descriptorPool);
	MVE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool");
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device.VulkanDevice(), descriptorPool, nullptr);
}

bool DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout,
										   VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool	 = descriptorPool;
	allocInfo.pSetLayouts		 = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
	// a new pool whenever an old pool fills up. But this is beyond our current scope
	if (vkAllocateDescriptorSets(device.VulkanDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
		return false;
	}
	return true;
}

void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
{
	vkFreeDescriptorSets(device.VulkanDevice(), descriptorPool, static_cast<uint32_t>(descriptors.size()),
						 descriptors.data());
}

void DescriptorPool::ResetPool()
{
	vkResetDescriptorPool(device.VulkanDevice(), descriptorPool, 0);
}

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool):
	setLayout {setLayout}, pool {pool}
{
}

DescriptorWriter& DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, uint32_t count)
{
	MVE_ASSERT(setLayout.bindings.count(binding) == 1, "Layout does not contain specified binding");

	auto& bindingDescription = setLayout.bindings[binding];

	MVE_ASSERT(bindingDescription.descriptorCount == count, "Binding {} descriptor info, but binding expects {}", count,
			   bindingDescription.descriptorCount);

	VkWriteDescriptorSet write {};
	write.sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType  = bindingDescription.descriptorType;
	write.dstBinding	  = binding;
	write.pBufferInfo	  = bufferInfo;
	write.descriptorCount = count;

	writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t count)
{
	MVE_ASSERT(setLayout.bindings.count(binding) == 1, "Layout does not contain specified binding");

	auto& bindingDescription = setLayout.bindings[binding];

	MVE_ASSERT(bindingDescription.descriptorCount == count, "Binding {} descriptor info, but binding expects {}", count,
			   bindingDescription.descriptorCount);

	VkWriteDescriptorSet write {};
	write.sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType  = bindingDescription.descriptorType;
	write.dstBinding	  = binding;
	write.pImageInfo	  = imageInfo;
	write.descriptorCount = count;

	writes.push_back(write);
	return *this;
}

bool DescriptorWriter::Build(VkDescriptorSet& set)
{
	bool success = pool.AllocateDescriptorSet(setLayout.GetDescriptorSetLayout(), set);
	if (!success) {
		return false;
	}
	Overwrite(set);
	return true;
}

void DescriptorWriter::Overwrite(VkDescriptorSet& set)
{
	for (auto& write : writes) { write.dstSet = set; }
	vkUpdateDescriptorSets(pool.device.VulkanDevice(), writes.size(), writes.data(), 0, nullptr);
}

} // namespace MVE