#pragma once

#include "Device.h"
#include "Model.h"

namespace MVE
{
struct PipelineConfigInfo
{
	PipelineConfigInfo()						  = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	void operator=(const PipelineConfigInfo&)	  = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescription {};
	std::vector<VkVertexInputAttributeDescription> attributeDescription {};

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass			= nullptr;
	uint32_t subpass				= 0;
};

class Pipeline
{
  public:
	Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath,
			 const PipelineConfigInfo& config);
	~Pipeline();

	Pipeline(const Pipeline&)		= delete;
	void operator=(const Pipeline&) = delete;

	void Bind(VkCommandBuffer commandBuffer);

  public:
	static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
	static void EnableAlphaBlending(PipelineConfigInfo& configInfo);

  private:
	void CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
								const PipelineConfigInfo& config);
	void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

  private:
	static std::vector<char> ReadFile(const std::string& filepath);

  private:
	Device& device;
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};
} // namespace MVE