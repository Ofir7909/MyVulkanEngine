#include "SkyboxSystem.h"

namespace MVE
{
SkyboxSystem::SkyboxSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout):
	device(device)
{
	CreatePipelineLayout(globalSetLayout);
	CreatePipeline(renderPass);
}

SkyboxSystem::~SkyboxSystem()
{
	vkDeviceWaitIdle(device.VulkanDevice());
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}

void SkyboxSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	std::vector<VkDescriptorSetLayout> descripotorSetLayouts {globalSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges	  = VK_NULL_HANDLE;

	auto error = vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	MVE_ASSERT(error == VK_SUCCESS, "Failed to create pipeline labs");
}

void SkyboxSystem::CreatePipeline(VkRenderPass renderPass)
{
	MVE_ASSERT(pipelineLayout != nullptr, "Pipeline can't be created before pipeline layout");

	PipelineConfigInfo pipelineConfig {};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	pipelineConfig.bindingDescription.clear();
	pipelineConfig.attributeDescription.clear();
	pipelineConfig.renderPass	  = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline					  = std::make_unique<Pipeline>(device, SHADER_BINARY_DIR "skybox.vert.spv",
										   SHADER_BINARY_DIR "skybox.frag.spv", pipelineConfig);
}

void SkyboxSystem::Render(FrameInfo& frameInfo, const Cubemap& cubemap)
{
	pipeline->Bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
							&frameInfo.globalDescriptorSet, 0, nullptr);

	vkCmdDraw(frameInfo.commandBuffer, 36, 1, 0, 0);
}

} // namespace MVE