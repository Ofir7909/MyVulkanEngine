#pragma once

#include "../Device.h"
#include "../FrameInfo.h"
#include "../Pipeline.h"
#include "moduels/Module.h"

#include "core/Application.h"
#include "core/GameObject.h"

namespace MVE
{

class PointLightSystem : public Module
{
  public:
	PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	void operator=(const PointLightSystem&)	  = delete;

	void Update(FrameInfo& frameInfo, GameObject::Map& gameObjects, GlobalUbo& ubo);
	void Render(FrameInfo& frameInfo, GameObject::Map& gameObjects);

  private:
	void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void CreatePipeline(VkRenderPass renderPass);

  private:
	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};
} // namespace MVE