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

float Input::GetAxis(KeyCode::KeyCode negative, KeyCode::KeyCode positive)
{
	return GetKey(positive) - GetKey(negative);
}

glm::vec2 Input::GetVector(KeyCode::KeyCode negativeX, KeyCode::KeyCode positiveX, KeyCode::KeyCode negativeY,
						   KeyCode::KeyCode positiveY)
{
	glm::vec2 vec = {GetAxis(negativeX, positiveX), GetAxis(negativeY, positiveY)};
	if (vec == glm::vec2 {})
		return vec;
	return glm::normalize(vec);
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