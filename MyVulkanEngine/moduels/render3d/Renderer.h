#pragma once
#include <vulkan/vulkan.h>

#include "Device.h"
#include "SwapChain.h"
#include "core/Application.h"
#include "moduels/Module.h"

namespace MVE
{
// TEMP
static VkExtent2D GetWindowExtent(Window& window)
{
	return {window.properties.width, window.properties.height};
}

class Renderer
{
  public:
	Renderer(Device& device);
	~Renderer();

	Renderer(const Renderer&)		= delete;
	void operator=(const Renderer&) = delete;

	VkRenderPass GetSwapChainRenderPass() const { return swapChain->GetRenderPass(); }
	float GetAspectRatio() const { return swapChain->ExtentAspectRatio(); }
	bool IsFrameInProgress() const { return isFrameStarted; }

	VkCommandBuffer GetCurrentCommandBuffer() const
	{
		MVE_ASSERT(isFrameStarted, "Cannot get command buffer when frame is not in progress");
		return commandBuffers[currentFrameIndex];
	}
	int GetFrameIndex() const
	{
		MVE_ASSERT(isFrameStarted, "Cannot get frame index when frame is not in progress");
		return currentFrameIndex;
	}

	VkCommandBuffer BeginFrame();
	void EndFrame();
	void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

  private:
	void CreateCommandBuffers();
	void FreeCommandBuffers();
	void RecreateSwapChain();

	void OnWindowResize(int width, int height);

  private:
	Device& device;
	std::unique_ptr<SwapChain> swapChain;
	std::vector<VkCommandBuffer> commandBuffers;

	uint32_t currentImageIndex;
	int currentFrameIndex;
	bool isFrameStarted = false;

	bool wasResized = false;
};
} // namespace MVE