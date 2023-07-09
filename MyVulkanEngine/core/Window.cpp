#include "Window.h"

namespace MVE
{

Window::Window(const WindowProperties& props): properties(props)
{
	glfwSetErrorCallback(
		[](int code, const char* description) { MVE_ERROR("GLFW Error! ({})\n{}", code, description); });

	MVE_ASSERT(glfwInit() == GLFW_TRUE, "Couldn't initialize GLFW!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	windowPtr = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
	// glfwSwapInterval(0);

	glfwSetWindowUserPointer(windowPtr, this);

	// Events

	glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* w, int width, int height) {
		auto window = static_cast<Window*>(glfwGetWindowUserPointer(w));

		window->properties.width  = width;
		window->properties.height = height;
		window->ResizeEvent(width, height);
	});
}

Window::~Window()
{
	glfwDestroyWindow(windowPtr);
	glfwTerminate();
}

void Window::OnUpdate()
{
	glfwPollEvents();
	// glfwSwapBuffers(windowPtr);
}

} // namespace MVE