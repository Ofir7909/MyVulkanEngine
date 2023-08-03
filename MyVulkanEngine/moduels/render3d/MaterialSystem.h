#pragma once

#include "Buffer.h"
#include "Descriptors.h"
#include "Texture.h"

namespace MVE
{
using MaterialId = uint32_t;

class MaterialSystem
{
  public:
	struct Params
	{
		glm::vec4 albedo   = glm::vec4 {1.0f};
		glm::vec4 emission = glm::vec4 {0.0f};
		glm::vec2 uvScale  = glm::vec2 {1.0f};
		float roughness	   = 0.5f;
		float metallic	   = 0.0f;
	};
	struct Textures
	{
		std::shared_ptr<Texture> albedo;
		std::shared_ptr<Texture> arm; // Ao - Roughness - Metalness
		std::shared_ptr<Texture> normal;
	};

	struct Material
	{
		Params params {};
		Textures textures {};
		std::vector<VkDescriptorSet> descriptorSet {};
		std::vector<std::unique_ptr<Buffer>> buffer {};
	};
	using MaterialsMap = std::unordered_map<MaterialId, Material>;

  public:
	MaterialSystem(Device& device);
	~MaterialSystem() {}

	MaterialId CreateMaterial();

	Material& Get(MaterialId id) { return materials.at(id); }
	void FlushMaterial(MaterialId id, int frameIndex);
	void FlushAll(int frameIndex);
	void Bind(MaterialId id, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int set,
			  int frameIndex) const;

	DescriptorSetLayout* GetLayout() { return setLayout.get(); }

  private:
	Device& device;
	MaterialsMap materials;

	std::unique_ptr<DescriptorPool> descriptorPool;
	std::unique_ptr<DescriptorSetLayout> setLayout;

	std::shared_ptr<Texture> defaultTextureAlbedo;
	std::shared_ptr<Texture> defaultTextureArm;
	std::shared_ptr<Texture> defaultTextureNormal;

	MaterialId defaultMaterialId = 0;
};
} // namespace MVE