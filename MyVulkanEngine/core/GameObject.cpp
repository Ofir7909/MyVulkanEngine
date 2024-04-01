#include "GameObject.h"

#include <glm/ext/matrix_transform.hpp>

namespace MVE
{
glm::mat4 TransformComponent::Mat4()
{
	return glm::translate(glm::toMat4(rotation) * glm::scale(glm::mat4 {1.0}, scale), translation);
}
glm::mat3 TransformComponent::NormalMatrix()
{
	return glm::transpose(glm::inverse(glm::mat3(Mat4())));
}
} // namespace MVE