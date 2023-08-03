#pragma once

#include "Device.h"

namespace MVE
{
class Texture
{
  public:
	static std::unique_ptr<Texture> Load(Device& device, const std::string& filepath);
	static VkImageView CreateImageView(Device& device, VkImage image, VkFormat format);
	static VkSampler createSampler(Device& device);

  public:
	Texture(Device& device): device(device) {}
	~Texture();

	VkImageView ImageView() const { return imageView; }
	VkSampler Sampler() const { return sampler; }
	VkDescriptorImageInfo ImageInfo() const;

  private:
	void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

  private:
	Device& device;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;
};
} // namespace MVE