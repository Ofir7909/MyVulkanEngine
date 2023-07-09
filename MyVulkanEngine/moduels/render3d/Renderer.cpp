#include "Renderer.h"

namespace MVE
{

Renderer::Renderer(Device& device): device(device)
{
	// Figue out what to do about deleting the event callback
	Application::Get()->GetWindow().ResizeEvent += new MemFuncEventCallback(this, &Renderer::OnWindowResize);

	RecreateSwapChain();
	CreateCommandBuffers();
}

Renderer::~Renderer()
{
	FreeCommandBuffers();
}

VkCommandBuffer Renderer::BeginFrame()
{
	assert(!isFrameStarted, "Frame already in progress");

	auto result = swapChain->AcquireNextImage(&currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		return nullptr;
	}

	MVE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed acquire swap chain image");

	isFrameStarted = true;

	auto commandBuffer = GetCurrentCommandBuffer();
	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	MVE_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS,
			   "Failed to begin recording command buffer");
	return commandBuffer;
}

void Renderer::EndFrame()
{
	assert(isFrameStarted, "Frame is not in progress");

	auto commandBuffer = GetCurrentCommandBuffer();
	MVE_ASSERT(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS, "Failed to end recording command buffer");

	auto result = swapChain->SubmitCommandBuffers(&commandBuffer, &currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || wasResized) {
		wasResized = false;
		RecreateSwapChain();
	} else {
		MVE_ASSERT(result == VK_SUCCESS, "Failed to submit command buffer");
	}

	isFrameStarted	  = false;
	currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted, "Frame is not in progress. Cant begin swap chain render pass");
	assert(commandBuffer == GetCurrentCommandBuffer(),
		   "get begin swap chain render pass on a command buffer from a diffrent frame.");

	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType	   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass  = swapChain->GetRenderPass();
	renderPassInfo.framebuffer = swapChain->GetFrameBuffer(currentImageIndex);

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChain->GetSwapChainExtent();

	std::array<VkClearValue, 2> clearValues {};
	clearValues[0].color		   = {0.1f, 0.1f, 0.1f, 1.0f};
	clearValues[1].depthStencil	   = {1.0f, 0};
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues	   = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport {};
	viewport.x		  = 0;
	viewport.y		  = 0;
	viewport.width	  = swapChain->GetSwapChainExtent().width;
	viewport.height	  = swapChain->GetSwapChainExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor {{0, 0}, swapChain->GetSwapChainExtent()};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted, "Frame is not in progress. Cant begin swap chain render pass");
	assert(commandBuffer == GetCurrentCommandBuffer(),
		   "get begin swap chain render pass on a command buffer from a diffrent frame.");

	vkCmdEndRenderPass(commandBuffer);
}

void Renderer::CreateCommandBuffers()
{
	commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool		 = device.GetCommandPool();
	allocInfo.commandBufferCount = commandBuffers.size();

	auto error = vkAllocateCommandBuffers(device.VulkanDevice(), &allocInfo, commandBuffers.data());
	MVE_ASSERT(error == VK_SUCCESS, "Failed to allocate command buffers");
}

void Renderer::FreeCommandBuffers()
{
	if (commandBuffers.size() == 0)
		return;
	vkFreeCommandBuffers(device.VulkanDevice(), device.GetCommandPool(), commandBuffers.size(), commandBuffers.data());
	commandBuffers.clear();
}

void Renderer::RecreateSwapChain()
{
	auto extent = GetWindowExtent(Application::Get()->GetWindow());
	while (extent.width == 0 || extent.height == 0) {
		extent = GetWindowExtent(Application::Get()->GetWindow());
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device.VulkanDevice());

	if (swapChain == nullptr) {
		swapChain = std::make_unique<SwapChain>(device, extent, std::move(swapChain));
	} else {
		std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
		swapChain								= std::make_unique<SwapChain>(device, extent, oldSwapChain);

		if (!oldSwapChain->CompareSwapFormats(*swapChain)) {
			MVE_ERROR("Swap chain image format has changed");
		}
	}
	// Used to create pipeline here. need to fix
}

void Renderer::OnWindowResize(int width, int height)
{
	wasResized = true;
}

} // namespace MVE