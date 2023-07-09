#include "Pipeline.h"

#include <fstream>

namespace MVE
{

Pipeline::Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath,
				   const PipelineConfigInfo& config):
	device(device)
{
	CreateGraphicsPipeline(vertFilepath, fragFilepath, config);
}

Pipeline ::~Pipeline()
{
	vkDestroyShaderModule(device.VulkanDevice(), vertShaderModule, nullptr);
	vkDestroyShaderModule(device.VulkanDevice(), fragShaderModule, nullptr);
	vkDestroyPipeline(device.VulkanDevice(), graphicsPipeline, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

void Pipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.inputAssemblyInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.inputAssemblyInfo.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	configInfo.inputAssemblyInfo.flags					= 0;

	configInfo.viewportInfo.sType		  = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.viewportInfo.viewportCount = 1;
	configInfo.viewportInfo.pViewports	  = nullptr;
	configInfo.viewportInfo.scissorCount  = 1;
	configInfo.viewportInfo.pScissors	  = nullptr;
	configInfo.viewportInfo.pNext		  = nullptr;
	configInfo.viewportInfo.flags		  = 0;

	configInfo.rasterizationInfo.sType					 = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.rasterizationInfo.depthClampEnable		 = VK_FALSE;
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.rasterizationInfo.polygonMode			 = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth				 = 1.0f;
	configInfo.rasterizationInfo.cullMode				 = VK_CULL_MODE_NONE;
	configInfo.rasterizationInfo.frontFace				 = VK_FRONT_FACE_CLOCKWISE;
	configInfo.rasterizationInfo.depthBiasEnable		 = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
	configInfo.rasterizationInfo.depthBiasClamp			 = 0.0f; // Optional
	configInfo.rasterizationInfo.depthBiasSlopeFactor	 = 0.0f; // Optional
	configInfo.rasterizationInfo.pNext					 = nullptr;
	configInfo.rasterizationInfo.flags					 = 0;

	configInfo.multisampleInfo.sType				 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable	 = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples	 = VK_SAMPLE_COUNT_1_BIT;
	configInfo.multisampleInfo.minSampleShading		 = 1.0f;	 // Optional
	configInfo.multisampleInfo.pSampleMask			 = nullptr;	 // Optional
	configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	configInfo.multisampleInfo.alphaToOneEnable		 = VK_FALSE; // Optional
	configInfo.multisampleInfo.pNext				 = nullptr;
	configInfo.multisampleInfo.flags				 = 0;

	configInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable			= VK_FALSE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	// Optional
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	configInfo.colorBlendAttachment.colorBlendOp		= VK_BLEND_OP_ADD;		// Optional
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	// Optional
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	configInfo.colorBlendAttachment.alphaBlendOp		= VK_BLEND_OP_ADD;		// Optional

	configInfo.colorBlendInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable		= VK_FALSE;
	configInfo.colorBlendInfo.logicOp			= VK_LOGIC_OP_COPY; // Optional
	configInfo.colorBlendInfo.attachmentCount	= 1;
	configInfo.colorBlendInfo.pAttachments		= &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

	configInfo.depthStencilInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable		  = VK_TRUE;
	configInfo.depthStencilInfo.depthWriteEnable	  = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp		  = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds		  = 0.0f; // Optional
	configInfo.depthStencilInfo.maxDepthBounds		  = 1.0f; // Optional
	configInfo.depthStencilInfo.stencilTestEnable	  = VK_FALSE;
	configInfo.depthStencilInfo.front				  = {}; // Optional
	configInfo.depthStencilInfo.back				  = {}; // Optional

	configInfo.dynamicStateEnables				  = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	configInfo.dynamicStateInfo.sType			  = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.dynamicStateInfo.pDynamicStates	  = configInfo.dynamicStateEnables.data();
	configInfo.dynamicStateInfo.dynamicStateCount = configInfo.dynamicStateEnables.size();
	configInfo.dynamicStateInfo.flags			  = 0;
	configInfo.dynamicStateInfo.pNext			  = nullptr;

	configInfo.bindingDescription	= Model::Vertex::getBindingDescriptions();
	configInfo.attributeDescription = Model::Vertex::getAttributeDescriptions();
}

void Pipeline::CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
									  const PipelineConfigInfo& config)
{
	auto vertCode = ReadFile(vertFilepath);
	auto fragCode = ReadFile(fragFilepath);

	MVE_INFO("Loaded Vetex Shader Code. Size: {}", vertCode.size());
	MVE_INFO("Loaded Fragment Shader Code. Size: {}", fragCode.size());

	CreateShaderModule(vertCode, &vertShaderModule);
	CreateShaderModule(fragCode, &fragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage				= VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module				= vertShaderModule;
	shaderStages[0].pName				= "main";
	shaderStages[0].flags				= 0;
	shaderStages[0].pNext				= nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module				= fragShaderModule;
	shaderStages[1].pName				= "main";
	shaderStages[1].flags				= 0;
	shaderStages[1].pNext				= nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	const auto& bindindDescriptions	  = config.bindingDescription;
	const auto& attributeDescriptions = config.attributeDescription;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.vertexBindingDescriptionCount	= bindindDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions		= bindindDescriptions.data();
	vertexInputInfo.pNext							= nullptr;
	vertexInputInfo.flags							= 0;

	VkGraphicsPipelineCreateInfo pipelineInfo {};
	pipelineInfo.sType				 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount			 = 2;
	pipelineInfo.pStages			 = shaderStages;
	pipelineInfo.pVertexInputState	 = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
	pipelineInfo.pViewportState		 = &config.viewportInfo;
	pipelineInfo.pRasterizationState = &config.rasterizationInfo;
	pipelineInfo.pMultisampleState	 = &config.multisampleInfo;
	pipelineInfo.pColorBlendState	 = &config.colorBlendInfo;
	pipelineInfo.pDepthStencilState	 = &config.depthStencilInfo;
	pipelineInfo.pDynamicState		 = &config.dynamicStateInfo;
	pipelineInfo.pTessellationState	 = nullptr;
	pipelineInfo.pNext				 = nullptr;
	pipelineInfo.flags				 = 0;

	pipelineInfo.layout		= config.pipelineLayout;
	pipelineInfo.renderPass = config.renderPass;
	pipelineInfo.subpass	= config.subpass;

	pipelineInfo.basePipelineIndex	= -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	auto error =
		vkCreateGraphicsPipelines(device.VulkanDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
	assert(error == VK_SUCCESS, "Failed to create graphics pipeline");
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo;
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode	= reinterpret_cast<const uint32_t*>(code.data());
	createInfo.flags	= 0;
	createInfo.pNext	= VK_NULL_HANDLE;

	auto error = vkCreateShaderModule(device.VulkanDevice(), &createInfo, nullptr, shaderModule);
	assert(error == VK_SUCCESS, "Failed to create shader module");
}

void Pipeline::EnableAlphaBlending(PipelineConfigInfo& configInfo)
{
	configInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable			= VK_TRUE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	configInfo.colorBlendAttachment.colorBlendOp		= VK_BLEND_OP_ADD;
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.alphaBlendOp		= VK_BLEND_OP_ADD;
}

std::vector<char> Pipeline::ReadFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

	MVE_ASSERT(file.is_open(), "Failed to open file at path: {}", filepath);

	int32_t filesize = file.tellg();
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer;
}

} // namespace MVE