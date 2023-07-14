#pragma once

#include "Buffer.h"
#include "Descriptors.h"

namespace MVE
{
struct PbrMaterial
{
	glm::vec4 albedo   = glm::vec4 {1.0f};
	glm::vec4 emission = glm::vec4 {0.0f};
	float roughness	   = 0.5f;
	float metallic	   = 0.0f;
};

using MaterialId = uint32_t;

class MaterialSystem
{
  public:
	struct MaterialInfo
	{
		PbrMaterial material {};
		VkDescriptorSet descriptorSet {};
		std::unique_ptr<Buffer> buffer {};
	};
	using MaterialsMap = std::unordered_map<MaterialId, MaterialInfo>;

  public:
	MaterialSystem(Device& device);
	~MaterialSystem() {}

	MaterialId CreateMaterial();

	PbrMaterial& Get(MaterialId id) { return materials.at(id).material; }
	void FlushMaterial(MaterialId id);
	void FlushAll();
	void Bind(MaterialId id, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int set) const;

	DescriptorSetLayout* GetLayout() { return setLayout.get(); }

  private:
	Device& device;
	MaterialsMap materials;

	std::unique_ptr<DescriptorPool> descriptorPool;
	std::unique_ptr<DescriptorSetLayout> setLayout;

	MaterialId defaultMaterialId = 0;
};
} // namespace MVE