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
		Builder(Device& device, const std::string& filepath): device_ {device}, path_ {filepath} {}

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
		std::unique_ptr<Texture> build();

	  private:
		VkImageView createImageView(VkImage image);
		VkSampler createSampler();

	  private:
		Device& device_;
		std::string path_;
		VkFormat format_				  = VK_FORMAT_R8G8B8A8_SRGB;
		VkFilter minMagFilter_			  = VK_FILTER_LINEAR;
		VkSamplerAddressMode addressMode_ = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	};

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