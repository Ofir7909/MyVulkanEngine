#include "MaterialSystem.h"
#include "FrameInfo.h"
#include "SwapChain.h"

constexpr int MAX_MATERIAL_COUNT = 100;

namespace MVE
{
MaterialSystem::MaterialSystem(Device& device): device(device)
{
	descriptorPool =
		DescriptorPool::Builder(device)
			.SetMaxSets(MAX_MATERIAL_COUNT * SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_MATERIAL_COUNT * SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						 MAX_MATERIAL_COUNT * SwapChain::MAX_FRAMES_IN_FLIGHT * 3)
			.Build();

	setLayout = DescriptorSetLayout::Builder(device)
					.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
					.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
					.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
					.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
					.Build();

	defaultTextureAlbedo = Texture::Load(device, RES_DIR "textures/white.jpg");
	defaultTextureArm	 = Texture::Load(device, RES_DIR "textures/white.jpg");
	defaultTextureNormal = Texture::Load(device, RES_DIR "textures/normal.jpg");

	// Default material
	defaultMaterialId = CreateMaterial();
}

MaterialId MaterialSystem::CreateMaterial()
{
	static MaterialId nextId = 0;

	MaterialId id = nextId++;
	Material mat {};
	mat.textures.albedo = defaultTextureAlbedo;
	mat.textures.arm	= defaultTextureArm;
	mat.textures.normal = defaultTextureNormal;
	materials.insert({id, std::move(mat)});

	materials[id].buffer.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	materials[id].descriptorSet.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
		materials[id].buffer[i] = std::make_unique<Buffer>(
			device, sizeof(Params), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			device.properties.limits.minUniformBufferOffsetAlignment);

		materials[id].buffer[i]->Map();

		auto bufferInfo		 = materials[id].buffer[i]->DescriptorInfo();
		auto albedoImageInfo = materials[id].textures.albedo->ImageInfo();
		auto armImageInfo	 = materials[id].textures.arm->ImageInfo();
		auto normalImageInfo = materials[id].textures.normal->ImageInfo();

		DescriptorWriter(*setLayout, *descriptorPool)
			.WriteBuffer(0, &bufferInfo)
			.WriteImage(1, &albedoImageInfo)
			.WriteImage(2, &armImageInfo)
			.WriteImage(3, &normalImageInfo)
			.Build(materials[id].descriptorSet[i]);
	}

	return id;
}

void MaterialSystem::FlushMaterial(MaterialId id, int frameIndex)
{
	auto& mat = materials[id];
	mat.buffer[frameIndex]->WriteToBuffer(&mat.params);

	auto bufferInfo		 = materials[id].buffer[frameIndex]->DescriptorInfo();
	auto albedoImageInfo = materials[id].textures.albedo->ImageInfo();
	auto armImageInfo	 = materials[id].textures.arm->ImageInfo();
	auto normalImageInfo = materials[id].textures.normal->ImageInfo();

	DescriptorWriter(*setLayout, *descriptorPool)
		.WriteBuffer(0, &bufferInfo)
		.WriteImage(1, &albedoImageInfo)
		.WriteImage(2, &armImageInfo)
		.WriteImage(3, &normalImageInfo)
		.Overwrite(materials[id].descriptorSet[frameIndex]);

	mat.buffer[frameIndex]->Flush();
}

void MaterialSystem::FlushAll(int frameIndex)
{
	for (auto& [id, mat] : materials) { FlushMaterial(id, frameIndex); }
}

void MaterialSystem::Bind(MaterialId id, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int set,
						  int frameIndex) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, set, 1,
							&(materials.at(id).descriptorSet[frameIndex]), 0, nullptr);
}

} // namespace MVE
