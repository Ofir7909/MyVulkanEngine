#pragma once

namespace MVE
{
class Camera
{
  public:
	Camera() {}
	~Camera() {}

	void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
	void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

	void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = {0.f, -1.f, 0.f});
	void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.f, -1.f, 0.f});
	void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

	const glm::mat4& GetProjection() const { return projMat; }
	const glm::mat4& GetView() const { return viewMat; }
	const glm::mat4& GetInverseView() const { return inverseViewMat; }
	const glm::vec3 GetPosition() const { return glm::vec3(inverseViewMat[3]); }

  private:
	glm::mat4 projMat {1.0f};
	glm::mat4 viewMat {1.0f};
	glm::mat4 inverseViewMat {1.0f};
};

} // namespace MVE