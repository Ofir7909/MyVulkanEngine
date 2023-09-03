#include "Cubemap.h"

#include "Camera.h"
#include "Descriptors.h"
#include "Pipeline.h"

MVE::Cubemap::Cubemap(Device& device, const std::string& folderPath, const std::string& extension): device(device)
{
	CreateTexture(folderPath, extension);
	// GenerateIrradiance();
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

void MVE::Cubemap::CreateFromHdri(const std::string& filepath, uint32_t resolution)
{
	Texture::Builder builder(device);

	builder.isCubemap(true)
		.format(VK_FORMAT_R8G8B8A8_UNORM)
		.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		.addUsageFlag(VK_IMAGE_USAGE_STORAGE_BIT)
		.layout(VK_IMAGE_LAYOUT_GENERAL);

	for (int i = 0; i < 6; i++) { builder.addLayer(SolidTextureSource(glm::vec4 {1.0f}, resolution, resolution)); }

	texture = builder.build();

	Equirect2Cubemap(filepath);
}

void MVE::Cubemap::Equirect2Cubemap(const std::string& filepath)
{
	auto hdri = Texture::Builder(device)
					.format(VK_FORMAT_R8G8B8A8_SRGB)
					.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
					.addLayer(FileTextureSource(filepath))
					.build();

	auto descriptorPool = DescriptorPool::Builder(device)
							  .SetMaxSets(1)
							  .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
							  .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
							  .Build();

	auto setLayout = DescriptorSetLayout::Builder(device)
						 .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
						 .AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
						 .Build();

	auto hdriInfo	 = hdri->ImageInfo();
	auto textureInfo = texture->ImageInfo();

	VkDescriptorSet set;
	DescriptorWriter(*setLayout, *descriptorPool).WriteImage(0, &hdriInfo).WriteImage(1, &textureInfo).Build(set);

	// create pipeline
	std::unique_ptr<ComputePipeline> pipeline;
	VkPipelineLayout pipelineLayout;

	std::vector<VkDescriptorSetLayout> descripotorSetLayouts {setLayout->GetDescriptorSetLayout()};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges	  = VK_NULL_HANDLE;

	vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	pipeline = std::make_unique<ComputePipeline>(device, SHADER_BINARY_DIR "equirect2cube.comp.spv", pipelineLayout);

	// render
	auto commandBuffer = device.BeginSingleTimeCommands();

	pipeline->Bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr);

	const glm::ivec3 shaderLocalSize {32, 32, 1};
	vkCmdDispatch(commandBuffer, texture->width() / shaderLocalSize.x, texture->height() / shaderLocalSize.y,
				  texture->layers() / shaderLocalSize.z);

	device.EndSingleTimeCommands(commandBuffer);
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}
