#pragma once

#include "Texture.h"

namespace MVE
{
class Cubemap
{
  public:
	Cubemap(Device& device, const std::string& folderPath, const std::string& extension);

	~Cubemap() {}

	VkDescriptorImageInfo ImageInfo() const { return texture->ImageInfo(); };

  private:
	void CreateTexture(const std::string& folderPath, const std::string& extension = "png");

  private:
	Device& device;
	std::shared_ptr<Texture> texture;
};

} // namespace MVE