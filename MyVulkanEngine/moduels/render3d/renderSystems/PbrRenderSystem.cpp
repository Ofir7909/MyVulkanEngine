#include "PbrRenderSystem.h"

namespace MVE
{
// TEMP
struct SimplePushConstantData
{
	glm::mat4 modelMatrix {1.0f};
	glm::mat4 normalMatrix {1.0f};
};

PbrRenderSystem::PbrRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
								 MaterialSystem& materialSystem):
	device(device)
{
	CreatePipelineLayout(globalSetLayout, materialSystem);
	CreatePipeline(renderPass);
}

PbrRenderSystem::~PbrRenderSystem()
{
	vkDeviceWaitIdle(device.VulkanDevice());
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}

void PbrRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout, MaterialSystem& materialSystem)
{
	VkPushConstantRange pushRange {};
	pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushRange.offset	 = 0;
	pushRange.size		 = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts {globalSetLayout,
															 materialSystem.GetLayout()->GetDescriptorSetLayout()};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts			  = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges	  = &pushRange;

	auto error = vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	MVE_ASSERT(error == VK_SUCCESS, "Failed to create pipeline labs");
}

void PbrRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	MVE_ASSERT(pipelineLayout != nullptr, "Pipeline can't be created before pipeline layout");

	PipelineConfigInfo pipelineConfig {};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass	  = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline					  = std::make_unique<GraphicsPipeline>(device, SHADER_BINARY_DIR "pbr.vert.spv",
												   SHADER_BINARY_DIR "pbr.frag.spv", pipelineConfig);
}

void PbrRenderSystem::RenderGameObjects(FrameInfo& frameInfo, GameObject::Map& gameObjects,
										MaterialSystem& materialSystem)
{
	pipeline->Bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
							&frameInfo.globalDescriptorSet, 0, nullptr);

	materialSystem.FlushAll(frameInfo.frameIndex);

	for (auto& [id, go] : gameObjects) {
		if (go.model == nullptr)
			continue;

		materialSystem.Bind(go.materialId, frameInfo.commandBuffer, pipelineLayout, 1, frameInfo.frameIndex);

		SimplePushConstantData push {};
		push.modelMatrix  = go.transform.Mat4();
		push.normalMatrix = go.transform.NormalMatrix();

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
						   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push), &push);

		go.model->Bind(frameInfo.commandBuffer);
		go.model->Draw(frameInfo.commandBuffer);
	}
}

} // namespace MVE