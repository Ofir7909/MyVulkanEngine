#pragma once

#include "../Cubemap.h"
#include "../Device.h"
#include "../FrameInfo.h"
#include "../Pipeline.h"
#include "moduels/Module.h"

#include "core/Application.h"
#include "core/GameObject.h"

namespace MVE
{

class SkyboxSystem : public Module
{
  public:
	SkyboxSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~SkyboxSystem();

	SkyboxSystem(const SkyboxSystem&)	= delete;
	void operator=(const SkyboxSystem&) = delete;

	void Render(FrameInfo& frameInfo, const Cubemap& cubemap);

  private:
	void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void CreatePipeline(VkRenderPass renderPass);

  private:
	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};
} // namespace MVE