#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace MVE
{
struct WindowProperties
{
	std::string title = "My Vulkan Engine";
	uint32_t width	  = 1280;
	uint32_t height	  = 720;
};

class Window
{
  public:
	Window(const WindowProperties& props = WindowProperties());
	~Window();
	Window(const Window&)			 = delete;
	Window& operator=(const Window&) = delete;

	void OnUpdate();

	bool ShouldClose() { return glfwWindowShouldClose(windowPtr); }

	GLFWwindow* GetNativeWindow() const { return windowPtr; }

  public:
	Event<int, int> ResizeEvent;

  public:
	WindowProperties properties;

  private:
	GLFWwindow* windowPtr;
};
} // namespace MVE
