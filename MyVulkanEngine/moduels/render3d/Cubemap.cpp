#include "Cubemap.h"

MVE::Cubemap::Cubemap(Device& device, const std::string& folderPath, const std::string& extension): device(device)
{
	CreateTexture(folderPath, extension);
}

void MVE::Cubemap::CreateTexture(const std::string& folderPath, const std::string& extension)
{
	std::string names[] = {"right", "left", "bottom", "top", "front", "back"};

	Texture::Builder builder(device);
	builder.isCubemap(true)
		.format(VK_FORMAT_R8G8B8A8_SRGB)
		.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		.useMipmaps(true);

	for (int i = 0; i < 6; i++) { builder.addLayer(FileTextureSource(folderPath + names[i] + "." + extension)); }

	texture = builder.build();
}
