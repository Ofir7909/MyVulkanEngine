#include "Camera.h"

namespace MVE
{
void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
	projMat		  = glm::mat4 {1.0f};
	projMat[0][0] = 2.f / (right - left);
	projMat[1][1] = 2.f / (bottom - top);
	projMat[2][2] = 1.f / (far - near);
	projMat[3][0] = -(right + left) / (right - left);
	projMat[3][1] = -(bottom + top) / (bottom - top);
	projMat[3][2] = -near / (far - near);
}
void Camera::SetPerspectiveProjection(float fovy, float aspect, float near, float far)
{
	MVE_ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f, "aspect is too samll");
	const float tanHalfFovy = tan(fovy / 2.f);

	projMat		  = glm::mat4 {0.0f};
	projMat[0][0] = 1.f / (aspect * tanHalfFovy);
	projMat[1][1] = 1.f / (tanHalfFovy);
	projMat[2][2] = far / (far - near);
	projMat[2][3] = 1.f;
	projMat[3][2] = -(far * near) / (far - near);
}
void Camera::SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	const glm::vec3 w {glm::normalize(direction)};
	const glm::vec3 u {glm::normalize(glm::cross(w, up))};
	const glm::vec3 v {glm::cross(w, u)};

	viewMat		  = glm::mat4 {1.f};
	viewMat[0][0] = u.x;
	viewMat[1][0] = u.y;
	viewMat[2][0] = u.z;
	viewMat[0][1] = v.x;
	viewMat[1][1] = v.y;
	viewMat[2][1] = v.z;
	viewMat[0][2] = w.x;
	viewMat[1][2] = w.y;
	viewMat[2][2] = w.z;
	viewMat[3][0] = -glm::dot(u, position);
	viewMat[3][1] = -glm::dot(v, position);
	viewMat[3][2] = -glm::dot(w, position);

	inverseViewMat		 = glm::mat4 {1.f};
	inverseViewMat[0][0] = u.x;
	inverseViewMat[0][1] = u.y;
	inverseViewMat[0][2] = u.z;
	inverseViewMat[1][0] = v.x;
	inverseViewMat[1][1] = v.y;
	inverseViewMat[1][2] = v.z;
	inverseViewMat[2][0] = w.x;
	inverseViewMat[2][1] = w.y;
	inverseViewMat[2][2] = w.z;
	inverseViewMat[3][0] = position.x;
	inverseViewMat[3][1] = position.y;
	inverseViewMat[3][2] = position.z;
}
void Camera::SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
	SetViewDirection(position, target - position, up);
}
void Camera::SetViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u {(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
	const glm::vec3 v {(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
	const glm::vec3 w {(c2 * s1), (-s2), (c1 * c2)};
	viewMat		  = glm::mat4 {1.f};
	viewMat[0][0] = u.x;
	viewMat[1][0] = u.y;
	viewMat[2][0] = u.z;
	viewMat[0][1] = v.x;
	viewMat[1][1] = v.y;
	viewMat[2][1] = v.z;
	viewMat[0][2] = w.x;
	viewMat[1][2] = w.y;
	viewMat[2][2] = w.z;
	viewMat[3][0] = -glm::dot(u, position);
	viewMat[3][1] = -glm::dot(v, position);
	viewMat[3][2] = -glm::dot(w, position);

	inverseViewMat		 = glm::mat4 {1.f};
	inverseViewMat[0][0] = u.x;
	inverseViewMat[0][1] = u.y;
	inverseViewMat[0][2] = u.z;
	inverseViewMat[1][0] = v.x;
	inverseViewMat[1][1] = v.y;
	inverseViewMat[1][2] = v.z;
	inverseViewMat[2][0] = w.x;
	inverseViewMat[2][1] = w.y;
	inverseViewMat[2][2] = w.z;
	inverseViewMat[3][0] = position.x;
	inverseViewMat[3][1] = position.y;
	inverseViewMat[3][2] = position.z;
}
} // namespace MVE