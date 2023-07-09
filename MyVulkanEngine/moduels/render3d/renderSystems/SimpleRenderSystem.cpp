#include "SimpleRenderSystem.h"

namespace MVE
{
// TEMP
struct SimplePushConstantData
{
	glm::mat4 modelMatrix {1.0f};
	glm::mat4 normalMatrix {1.0f};
};

SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout):
	device(device)
{
	CreatePipelineLayout(globalSetLayout);
	CreatePipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem()
{
	vkDeviceWaitIdle(device.VulkanDevice());
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	VkPushConstantRange pushRange {};
	pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushRange.offset	 = 0;
	pushRange.size		 = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descripotorSetLayouts {globalSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges	  = &pushRange;

	auto error = vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	MVE_ASSERT(error == VK_SUCCESS, "Failed to create pipeline labs");
}

void SimpleRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	MVE_ASSERT(pipelineLayout != nullptr, "Pipeline can't be created before pipeline layout");

	PipelineConfigInfo pipelineConfig {};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass	  = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline					  = std::make_unique<Pipeline>(device, SHADER_BINARY_DIR "simple.vert.spv",
										   SHADER_BINARY_DIR "simple.frag.spv", pipelineConfig);
}

void SimpleRenderSystem::RenderGameObjects(FrameInfo& frameInfo, GameObject::Map& gameObjects)
{
	pipeline->Bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
							&frameInfo.globalDescriptorSet, 0, nullptr);

	for (auto& [id, go] : gameObjects) {
		if (go.model == nullptr)
			continue;

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