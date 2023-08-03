#include "MaterialSystem.h"
#include "FrameInfo.h"

namespace MVE
{

MaterialSystem::MaterialSystem(Device& device): device(device)
{
	descriptorPool =
		DescriptorPool::Builder(device).SetMaxSets(100).AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100).Build();

	setLayout = DescriptorSetLayout::Builder(device)
					.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
					.Build();

	// Default material
	defaultMaterialId = CreateMaterial();
}

MaterialId MaterialSystem::CreateMaterial()
{
	static MaterialId nextId = 0;

	auto id = nextId++;
	materials.emplace(id, Material {});

	materials[id].buffer = std::make_unique<Buffer>(device, sizeof(Params), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
													VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
													device.properties.limits.minUniformBufferOffsetAlignment);
	materials[id].buffer->Map();

	auto bufferInfo = materials[id].buffer->DescriptorInfo();
	DescriptorWriter(*setLayout, *descriptorPool).WriteBuffer(0, &bufferInfo).Build(materials[id].descriptorSet);

	return id;
}

void MaterialSystem::FlushMaterial(MaterialId id)
{
	auto& mat = materials[id];
	mat.buffer->WriteToBuffer(&mat.params);
	mat.buffer->Flush();
}

void MaterialSystem::FlushAll()
{
	for (auto& [id, mat] : materials) { FlushMaterial(id); }
}

void MaterialSystem::Bind(MaterialId id, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int set) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, set, 1,
							&(materials.at(id).descriptorSet), 0, nullptr);
}

} // namespace MVE
