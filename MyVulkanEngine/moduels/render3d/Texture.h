#pragma once

#include "Device.h"

namespace MVE
{
class TextureSource
{
	friend class Texture;

  public:
	virtual uint32_t width() { return width_; }
	virtual uint32_t height() { return height_; }
	virtual uint32_t bpp() { return bpp_; }

  protected:
	TextureSource() {}

  protected:
	std::vector<uint8_t> pixels_;
	uint32_t width_;
	uint32_t height_;
	uint32_t bpp_;
};

class FileTextureSource : public TextureSource
{
  public:
	FileTextureSource(const std::string& filepath);
};

class SolidTextureSource : public TextureSource
{
  public:
	SolidTextureSource(glm::vec4 color, uint32_t width = 1, uint32_t height = 1);
};

class Texture
{
  public:
	class Builder
	{
	  public:
		Builder(Device& device): device_ {device} {}

		Builder& addLayer(TextureSource&& source);

		Builder& format(VkFormat format)
		{
			format_ = format;
			return *this;
		}
		Builder& filter(VkFilter filter)
		{
			minMagFilter_ = filter;
			return *this;
		}
		Builder& addressMode(VkSamplerAddressMode mode)
		{
			addressMode_ = mode;
			return *this;
		}
		Builder& isCubemap(bool value)
		{
			isCubemap_ = value;
			return *this;
		}
		Builder& useMipmaps(bool use, VkSamplerMipmapMode mode = VK_SAMPLER_MIPMAP_MODE_LINEAR)
		{
			useMipmaps_ = use;
			mipmapMode_ = mode;
			return *this;
		}

		std::unique_ptr<Texture> build();

	  private:
		void generateMipmaps();
		VkImageView createImageView();
		VkSampler createSampler();

	  private:
		Device& device_;
		std::unique_ptr<Texture> image_;
		std::vector<std::vector<uint8_t>> layers_;
		uint32_t width_	 = -1;
		uint32_t height_ = -1;
		uint32_t bpp_	 = -1;

		VkFormat format_				  = VK_FORMAT_R8G8B8A8_SRGB;
		VkFilter minMagFilter_			  = VK_FILTER_LINEAR;
		VkSamplerAddressMode addressMode_ = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		bool isCubemap_					  = false;
		bool useMipmaps_				  = false;
		VkSamplerMipmapMode mipmapMode_	  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		int mipmapCount_				  = 1;
	};

  public:
	Texture(Device& device): device(device) {}
	~Texture();

	VkImageView ImageView() const { return imageView; }
	VkSampler Sampler() const { return sampler; }
	VkDescriptorImageInfo ImageInfo() const;

  private:
	void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount,
							   uint32_t mipmapCount);

  private:
	Device& device;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;
};
} // namespace MVE