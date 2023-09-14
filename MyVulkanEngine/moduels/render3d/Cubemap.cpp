#include "Cubemap.h"

#include "Camera.h"
#include "Descriptors.h"
#include "Pipeline.h"

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
	GenerateIBL();
}

void MVE::Cubemap::CreateFromHdri(const std::string& filepath, uint32_t resolution)
{
	Texture::Builder builder(device);

	builder.isCubemap(true)
		.format(VK_FORMAT_R32G32B32A32_SFLOAT)
		.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		.addUsageFlag(VK_IMAGE_USAGE_STORAGE_BIT)
		.layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.useMipmaps(true);

	for (int i = 0; i < 6; i++) { builder.addLayer(FloatSolidTextureSource(glm::vec4 {1.0f}, resolution, resolution)); }

	texture = builder.build();
	texture->TransitionImageLayout(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								   VK_IMAGE_LAYOUT_GENERAL, 6, texture->mipMaps());

	Equirect2Cubemap(filepath);
	GenerateIBL();
}

void MVE::Cubemap::Equirect2Cubemap(const std::string& filepath)
{
	auto hdri = Texture::Builder(device)
					.format(VK_FORMAT_R32G32B32A32_SFLOAT)
					.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
					.addLayer(FloatFileTextureSource(filepath))
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

void MVE::Cubemap::GenerateIBL(uint32_t irradianceResolution)
{
	GenerateIrradiance(irradianceResolution);
	PrefilterMap();
}

void MVE::Cubemap::GenerateIrradiance(uint32_t resolution)
{
	Texture::Builder builder(device);
	builder.isCubemap(true)
		.format(VK_FORMAT_R32G32B32A32_SFLOAT)
		.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
		.addUsageFlag(VK_IMAGE_USAGE_STORAGE_BIT)
		.layout(VK_IMAGE_LAYOUT_GENERAL);

	for (int i = 0; i < 6; i++) { builder.addLayer(FloatSolidTextureSource(glm::vec4 {1.0f}, resolution, resolution)); }
	irradiance = builder.build();

	auto descriptorPool = DescriptorPool::Builder(device)
							  .SetMaxSets(1)
							  .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
							  .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
							  .Build();

	auto setLayout = DescriptorSetLayout::Builder(device)
						 .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
						 .AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
						 .Build();

	auto textureInfo	= texture->ImageInfo();
	auto irradianceInfo = irradiance->ImageInfo();

	VkDescriptorSet set;
	DescriptorWriter(*setLayout, *descriptorPool).WriteImage(0, &textureInfo).WriteImage(1, &irradianceInfo).Build(set);

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

	pipeline =
		std::make_unique<ComputePipeline>(device, SHADER_BINARY_DIR "irradianceGenerator.comp.spv", pipelineLayout);

	// render
	auto commandBuffer = device.BeginSingleTimeCommands();

	pipeline->Bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr);

	const glm::ivec3 shaderLocalSize {16, 16, 1};
	vkCmdDispatch(commandBuffer, texture->width() / shaderLocalSize.x, texture->height() / shaderLocalSize.y,
				  texture->layers() / shaderLocalSize.z);

	device.EndSingleTimeCommands(commandBuffer);
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}

void MVE::Cubemap::PrefilterMap()
{
	uint32_t levels		= texture->mipMaps();
	auto descriptorPool = DescriptorPool::Builder(device)
							  .SetMaxSets(1)
							  .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
							  .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, levels - 1)
							  .Build();

	auto setLayout = DescriptorSetLayout::Builder(device)
						 .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
						 .AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, levels - 1)
						 .Build();

	auto textureInfo = texture->ImageInfo();
	std::vector<VkDescriptorImageInfo> mipMapsImageInfos;

	VkImageViewCreateInfo createInfo {};
	createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image						   = texture->image;
	createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_CUBE;
	createInfo.format						   = texture->format_;
	createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount	   = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount	   = texture->layers_;

	VkDescriptorImageInfo imageInfo {};
	imageInfo.imageLayout = texture->layout_;
	imageInfo.sampler	  = nullptr;

	for (int i = 1; i < levels; i++) {
		createInfo.subresourceRange.baseMipLevel = i;

		VkImageView imageView;
		auto code = vkCreateImageView(device.VulkanDevice(), &createInfo, nullptr, &imageView);
		MVE_ASSERT(code == VK_SUCCESS, "Failed to create image view");

		imageInfo.imageView = imageView;

		mipMapsImageInfos.push_back(imageInfo);
	}

	VkDescriptorSet set;
	DescriptorWriter(*setLayout, *descriptorPool)
		.WriteImage(0, &textureInfo)
		.WriteImage(1, mipMapsImageInfos.data(), levels - 1)
		.Build(set);

	struct
	{
		int level;
		float roughness;
	} pushConstants;

	// create pipeline
	std::unique_ptr<ComputePipeline> pipeline;
	VkPipelineLayout pipelineLayout;

	std::vector<VkDescriptorSetLayout> descripotorSetLayouts {setLayout->GetDescriptorSetLayout()};

	VkPushConstantRange pushRange {};
	pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	pushRange.offset	 = 0;
	pushRange.size		 = sizeof(pushConstants);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges	  = &pushRange;

	const uint32_t specializationData[] = {levels - 1};
	VkSpecializationMapEntry specializationMap {};
	specializationMap.constantID = 0;
	specializationMap.offset	 = 0;
	specializationMap.size		 = sizeof(uint32_t);

	VkSpecializationInfo specializationInfo {};
	specializationInfo.mapEntryCount = 1;
	specializationInfo.pData		 = specializationData;
	specializationInfo.pMapEntries	 = &specializationMap;
	specializationInfo.dataSize		 = sizeof(specializationData);

	vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	pipeline = std::make_unique<ComputePipeline>(device, SHADER_BINARY_DIR "prefilterSkybox.comp.spv", pipelineLayout,
												 &specializationInfo);

	// render
	auto commandBuffer = device.BeginSingleTimeCommands();
	pipeline->Bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr);

	uint32_t width	= texture->width() / 2;
	uint32_t height = texture->height() / 2;
	for (int level = 1; level < levels; level++) {
		pushConstants.level		= level - 1;
		pushConstants.roughness = (float)level / (levels - 1);

		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants),
						   &pushConstants);

		const glm::ivec3 shaderLocalSize {16, 16, 1};
		vkCmdDispatch(commandBuffer, width / shaderLocalSize.x, height / shaderLocalSize.y,
					  texture->layers() / shaderLocalSize.z);

		width  = std::max<uint32_t>(width / 2, shaderLocalSize.x);
		height = std::max<uint32_t>(height / 2, shaderLocalSize.y);
	}

	device.EndSingleTimeCommands(commandBuffer);
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);

	for (auto& info : mipMapsImageInfos) { vkDestroyImageView(device.VulkanDevice(), info.imageView, nullptr); }
}