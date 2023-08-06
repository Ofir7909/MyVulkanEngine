#pragma once

#include "Device.h"

namespace MVE
{
class Texture
{
  public:
	class Builder
	{
	  public:
		Builder(Device& device): device_ {device} {}

		Builder addLayer(const std::string& filepath);

		Builder format(VkFormat format)
		{
			format_ = format;
			return *this;
		}
		Builder filter(VkFilter filter)
		{
			minMagFilter_ = filter;
			return *this;
		}
		Builder addressMode(VkSamplerAddressMode mode)
		{
			addressMode_ = mode;
			return *this;
		}
		Builder isCubemap(bool value)
		{
			isCubemap_ = value;
			return *this;
		}

		std::unique_ptr<Texture> build();

	  private:
		VkImageView createImageView(VkImage image);
		VkSampler createSampler();

	  private:
		Device& device_;
		std::vector<std::string> layers_;
		VkFormat format_				  = VK_FORMAT_R8G8B8A8_SRGB;
		VkFilter minMagFilter_			  = VK_FILTER_LINEAR;
		VkSamplerAddressMode addressMode_ = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		bool isCubemap_					  = false;
	};

  public:
	Texture(Device& device): device(device) {}
	~Texture();

	VkImageView ImageView() const { return imageView; }
	VkSampler Sampler() const { return sampler; }
	VkDescriptorImageInfo ImageInfo() const;

  private:
	void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount);

  private:
	Device& device;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;
};
} // namespace MVE