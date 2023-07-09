#pragma once

#include "Device.h"

namespace MVE
{

class DescriptorSetLayout
{
  public:
	class Builder
	{
	  public:
		Builder(Device& device): device {device} {}

		Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
							uint32_t count = 1);
		std::unique_ptr<DescriptorSetLayout> Build() const;

	  private:
		Device& device;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings {};
	};

  public:
	DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~DescriptorSetLayout();
	DescriptorSetLayout(const DescriptorSetLayout&)			   = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

  private:
	Device& device;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

	friend class DescriptorWriter;
};

class DescriptorPool
{
  public:
	class Builder
	{
	  public:
		Builder(Device& device): device {device} {}

		Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& SetMaxSets(uint32_t count);
		std::unique_ptr<DescriptorPool> Build() const;

	  private:
		Device& device;
		std::vector<VkDescriptorPoolSize> poolSizes {};
		uint32_t maxSets					  = 1000;
		VkDescriptorPoolCreateFlags poolFlags = 0;
	};

	DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
				   const std::vector<VkDescriptorPoolSize>& poolSizes);
	~DescriptorPool();
	DescriptorPool(const DescriptorPool&)			 = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

	void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

	void ResetPool();

  private:
	Device& device;
	VkDescriptorPool descriptorPool;

	friend class DescriptorWriter;
};

class DescriptorWriter
{
  public:
	DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

	DescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
	DescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

	bool Build(VkDescriptorSet& set);
	void Overwrite(VkDescriptorSet& set);

  private:
	DescriptorSetLayout& setLayout;
	DescriptorPool& pool;
	std::vector<VkWriteDescriptorSet> writes;
};

} // namespace MVE