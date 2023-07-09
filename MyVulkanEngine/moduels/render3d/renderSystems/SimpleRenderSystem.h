#pragma once

#include "../Device.h"
#include "../FrameInfo.h"
#include "../Pipeline.h"
#include "moduels/Module.h"

#include "core/Application.h"
#include "core/GameObject.h"

namespace MVE
{

class SimpleRenderSystem : public Module
{
  public:
	SimpleRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~SimpleRenderSystem();

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	void operator=(const SimpleRenderSystem&)	  = delete;

	void RenderGameObjects(FrameInfo& frameInfo, GameObject::Map& gameObjects);

  private:
	void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void CreatePipeline(VkRenderPass renderPass);

  private:
	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};
} // namespace MVE