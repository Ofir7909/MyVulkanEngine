#pragma once

#include <vulkan/Vulkan.h>

#include "Camera.h"

namespace MVE
{

#define MAX_LIGHTS 10

struct PointLight
{
	glm::vec4 position {};
	glm::vec4 color {};
};

struct GlobalUbo
{
	glm::mat4 view {1.0f};
	glm::mat4 projection {1.0f};
	glm::mat4 inverseView {1.0f};
	glm::vec4 ambientLightColor {0.6f, 0.6f, 1.0f, 0.02f};
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};

struct FrameInfo
{
	int frameIndex;
	float deltaTime;
	VkCommandBuffer commandBuffer;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet;
};
} // namespace MVE