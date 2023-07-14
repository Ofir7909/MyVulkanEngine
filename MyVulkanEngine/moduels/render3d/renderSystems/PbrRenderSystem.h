#pragma once

#include "../Device.h"
#include "../FrameInfo.h"
#include "../MaterialSystem.h"
#include "../Pipeline.h"
#include "moduels/Module.h"

#include "core/Application.h"
#include "core/GameObject.h"

namespace MVE
{

class PbrRenderSystem : public Module
{
  public:
	PbrRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
					MaterialSystem& materialSystem);
	~PbrRenderSystem();

	PbrRenderSystem(const PbrRenderSystem&) = delete;
	void operator=(const PbrRenderSystem&)	= delete;

	void RenderGameObjects(FrameInfo& frameInfo, GameObject::Map& gameObjects, MaterialSystem& materialSystem);

  private:
	void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout, MaterialSystem& materialSystem);
	void CreatePipeline(VkRenderPass renderPass);

  private:
	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};
} // namespace MVE