#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Buffer.h"

namespace MVE
{
std::unique_ptr<Texture> Texture::Builder::build()
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(path_.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	VkDeviceSize imageSize = width * height * 4;

	MVE_ASSERT(pixels, "Failed to load image from path {}", path_);

	Buffer buffer(device_, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	buffer.Map();
	buffer.WriteToBuffer(pixels);
	buffer.Flush();

	stbi_image_free(pixels);

	VkImageCreateInfo createInfo {};
	createInfo.sType		 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType	 = VK_IMAGE_TYPE_2D;
	createInfo.extent.width	 = width;
	createInfo.extent.height = height;
	createInfo.extent.depth	 = 1;
	createInfo.mipLevels	 = 1;
	createInfo.arrayLayers	 = 1;
	createInfo.format		 = format_;
	createInfo.tiling		 = VK_IMAGE_TILING_OPTIMAL;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage		 = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples		 = VK_SAMPLE_COUNT_1_BIT;
	createInfo.flags		 = 0;

	auto image = std::make_unique<Texture>(device_);
	device_.CreateImageWithInfo(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image->image, image->imageMemory);

	image->TransitionImageLayout(format_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	device_.CopyBufferToImage(buffer.GetBuffer(), image->image, width, height, 1);

	image->TransitionImageLayout(format_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	image->imageView = createImageView(image->image);
	image->sampler	 = createSampler();

	return image;
}

VkImageView Texture::Builder::createImageView(VkImage image)
{
	VkImageViewCreateInfo createInfo {};
	createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image						   = image;
	createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format						   = format_;
	createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel   = 0;
	createInfo.subresourceRange.levelCount	   = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount	   = 1;

	VkImageView imageView;
	auto code = vkCreateImageView(device_.VulkanDevice(), &createInfo, nullptr, &imageView);
	MVE_ASSERT(code == VK_SUCCESS, "Failed to create image view");

	return imageView;
}

VkSampler Texture::Builder::createSampler()
{
	VkSampler sampler {};

	VkSamplerCreateInfo createInfo {};
	createInfo.sType				   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter			   = minMagFilter_;
	createInfo.minFilter			   = minMagFilter_;
	createInfo.addressModeU			   = addressMode_;
	createInfo.addressModeV			   = addressMode_;
	createInfo.addressModeW			   = addressMode_;
	createInfo.anisotropyEnable		   = VK_TRUE;
	createInfo.maxAnisotropy		   = device_.properties.limits.maxSamplerAnisotropy;
	createInfo.borderColor			   = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable		   = VK_FALSE;
	createInfo.compareOp			   = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode			   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias			   = 0.0f;
	createInfo.minLod				   = 0.0f;
	createInfo.maxLod				   = 0.0f;

	auto code = vkCreateSampler(device_.VulkanDevice(), &createInfo, nullptr, &sampler);
	MVE_ASSERT(code == VK_SUCCESS, "Failed to create sampler");

	return sampler;
}

Texture::~Texture()
{
	vkDestroySampler(device.VulkanDevice(), sampler, nullptr);
	vkDestroyImageView(device.VulkanDevice(), imageView, nullptr);
	vkDestroyImage(device.VulkanDevice(), image, nullptr);
	vkFreeMemory(device.VulkanDevice(), imageMemory, nullptr);
}

VkDescriptorImageInfo Texture::ImageInfo() const
{
	VkDescriptorImageInfo imageInfo {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView	  = imageView;
	imageInfo.sampler	  = sampler;
	return imageInfo;
}

void Texture::TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	auto commandBuffer = device.BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier {};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout						= oldLayout;
	barrier.newLayout						= newLayout;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;
	barrier.srcAccessMask					= 0; // TODO
	barrier.dstAccessMask					= 0; // TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	device.EndSingleTimeCommands(commandBuffer);
}
} // namespace MVE