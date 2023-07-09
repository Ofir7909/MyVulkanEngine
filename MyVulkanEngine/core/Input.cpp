#include "Input.h"

#include "Application.h"
#include <GLFW/glfw3.h>

namespace MVE
{
static GLFWwindow* GetGLFWwindow()
{
	return Application::Get()->GetWindow().GetNativeWindow();
}

bool Input::GetKey(KeyCode::KeyCode key)
{
	auto state = glfwGetKey(GetGLFWwindow(), key);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::GetMouseButton(int button)
{
	return glfwGetMouseButton(GetGLFWwindow(), button) == GLFW_PRESS;
}

glm::vec2 Input::GetMousePosition()
{
	double x, y;
	glfwGetCursorPos(GetGLFWwindow(), &x, &y);
	return {x, y};
}

} // namespace MVE