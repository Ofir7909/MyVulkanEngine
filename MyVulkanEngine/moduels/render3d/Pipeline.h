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
  protected:
	Pipeline(Device& device): device(device) {}

  public:
	virtual ~Pipeline() = default;

	Pipeline(const Pipeline&)		= delete;
	void operator=(const Pipeline&) = delete;

	virtual void Bind(VkCommandBuffer commandBuffer) = 0;

  public:
	static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
	static void EnableAlphaBlending(PipelineConfigInfo& configInfo);

  protected:
	void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

  protected:
	static std::vector<char> ReadFile(const std::string& filepath);

  protected:
	Device& device;
};

class GraphicsPipeline : public Pipeline
{
  public:
	GraphicsPipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath,
					 const PipelineConfigInfo& config);

	~GraphicsPipeline();

	void Bind(VkCommandBuffer commandBuffer) override;

  private:
	void CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
								const PipelineConfigInfo& config);

  private:
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};

class ComputePipeline : public Pipeline
{
  public:
	ComputePipeline(Device& device, const std::string& computeFilepath, VkPipelineLayout pipelineLayout,
					VkSpecializationInfo* specializationInfo = nullptr);

	~ComputePipeline();

	void Bind(VkCommandBuffer commandBuffer) override;

  private:
	void CreateComputePipeline(const std::string& computeFilepath, VkPipelineLayout pipelineLayout,
							   VkSpecializationInfo* specializationInfo);

  private:
	VkPipeline computePipeline;
	VkShaderModule computeShaderModule;
};

} // namespace MVE