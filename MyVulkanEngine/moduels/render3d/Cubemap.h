#pragma once

#include "Texture.h"

namespace MVE
{
class Cubemap
{
  public:
	Cubemap(Device& device): device(device) {}
	Cubemap(Device& device, const std::string& folderPath, const std::string& extension);

	~Cubemap() {}

	void CreateFromHdri(const std::string& filepath, uint32_t resolution = 512);

	VkDescriptorImageInfo ImageInfo() const { return texture->ImageInfo(); };

  private:
	void CreateTexture(const std::string& folderPath, const std::string& extension = "png");
	void Equirect2Cubemap(const std::string& filepath);

  private:
	Device& device;
	std::shared_ptr<Texture> texture;
};

} // namespace MVE