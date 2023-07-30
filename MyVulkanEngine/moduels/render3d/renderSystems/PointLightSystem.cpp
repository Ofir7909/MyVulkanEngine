#include "PointLightSystem.h"

namespace MVE
{

struct PointLightPushConstants
{
	glm::vec4 position {};
	glm::vec4 color {};
	float radius;
};

PointLightSystem::PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout):
	device(device)
{
	CreatePipelineLayout(globalSetLayout);
	CreatePipeline(renderPass);
}

PointLightSystem::~PointLightSystem()
{
	vkDeviceWaitIdle(device.VulkanDevice());
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}

void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	VkPushConstantRange pushRange {};
	pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushRange.offset	 = 0;
	pushRange.size		 = sizeof(PointLightPushConstants);

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

void PointLightSystem::CreatePipeline(VkRenderPass renderPass)
{
	MVE_ASSERT(pipelineLayout != nullptr, "Pipeline can't be created before pipeline layout");

	PipelineConfigInfo pipelineConfig {};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
	Pipeline::EnableAlphaBlending(pipelineConfig);

	pipelineConfig.bindingDescription.clear();
	pipelineConfig.attributeDescription.clear();
	pipelineConfig.renderPass	  = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline					  = std::make_unique<Pipeline>(device, SHADER_BINARY_DIR "point_light.vert.spv",
										   SHADER_BINARY_DIR "point_light.frag.spv", pipelineConfig);
}

void PointLightSystem::Update(FrameInfo& frameInfo, GameObject::Map& gameObjects, GlobalUbo& ubo)
{
	int i = 0;
	int j = 0;
	for (auto& [id, go] : gameObjects) {
		if (go.pointLight != nullptr) {
			ubo.pointLights[i].position = glm::vec4(go.transform.translation, 1.0f);
			ubo.pointLights[i].color	= glm::vec4(go.color, go.pointLight->lightIntensity);
			i++;
		} else if (go.directionalLight != nullptr) {
			ubo.directionalLights[j].direction = glm::vec4(go.transform.Mat4()[0]);
			ubo.directionalLights[j].color	   = glm::vec4(go.color, go.directionalLight->lightIntensity);
			j++;
		}
	};
	ubo.numPointLights		 = i;
	ubo.numDirectionalLights = j;
}

void PointLightSystem::Render(FrameInfo& frameInfo, GameObject::Map& gameObjects)
{
	// sort lights by distance from camera.
	std::multimap<float, GameObject::id_t> lightsDistance;
	glm::vec3 cameraPos = frameInfo.camera.GetPosition();
	for (auto& [id, go] : gameObjects) {
		if (go.pointLight == nullptr)
			continue;
		auto diff = go.transform.translation - cameraPos;
		lightsDistance.emplace(glm::dot(diff, diff), id);
	}

	pipeline->Bind(frameInfo.commandBuffer);

	for (auto it = lightsDistance.rbegin(); it != lightsDistance.rend(); it++) {
		auto& go = gameObjects.at(it->second);

		PointLightPushConstants pushConstants {};
		pushConstants.color	   = glm::vec4(go.color, go.pointLight->lightIntensity);
		pushConstants.position = glm::vec4(go.transform.translation, 1.0f);
		pushConstants.radius   = go.transform.scale.x;

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
						   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
						   sizeof(PointLightPushConstants), &pushConstants);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
								&frameInfo.globalDescriptorSet, 0, nullptr);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}
}

} // namespace MVE