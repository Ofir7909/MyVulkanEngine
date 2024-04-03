#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// TODO: remove model from game object into a seperate component.
#include "moduels/render3d/MaterialSystem.h"
#include "moduels/render3d/Model.h"

namespace MVE
{
struct TransformComponent
{
	glm::vec3 translation {};
	glm::vec3 scale {1.0f, 1.0f, 1.0f};
	glm::quat rotation {1.0f, 0.0f, 0.0f, 0.0f};

	glm::mat4 Mat4();
	glm::mat3 NormalMatrix();

	glm::vec3 Forward() { return glm::rotate(rotation, glm::vec3 {0.0, 0.0, -1.0}); }
	glm::vec3 Back() { return -Forward(); }
	glm::vec3 Right() { return glm::rotate(rotation, glm::vec3 {1.0, 0.0, 0.0}); }
	glm::vec3 Left() { return -Right(); }
	glm::vec3 Up() { return glm::rotate(rotation, glm::vec3 {0.0, 1.0, 0.0}); }
	glm::vec3 Down() { return -Up(); }
};

struct PointLightComponent
{
	float lightIntensity = 1.0f;
};

struct DirectionalLightComponent
{
	float lightIntensity = 1.0f;
};

class GameObject
{
  public:
	using id_t = uint32_t;
	using Map  = std::unordered_map<id_t, GameObject>;

  public:
	static GameObject Create()
	{
		static id_t nextId = 0;
		return GameObject(nextId++);
	}
	static GameObject CreatePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3 {1.0})
	{
		auto light			  = Create();
		light.pointLight	  = std::make_unique<PointLightComponent>(intensity);
		light.color			  = color;
		light.transform.scale = glm::vec3(radius);
		return light;
	}
	static GameObject CreateDirectionalLight(float intensity = 10.0f, glm::vec3 color = glm::vec3 {1.0})
	{
		auto light			   = Create();
		light.directionalLight = std::make_unique<DirectionalLightComponent>(intensity);
		light.color			   = color;
		return light;
	}

  private:
	GameObject(id_t objId): id(objId) {}

  public:
	GameObject(const GameObject&)		= delete;
	void operator=(const GameObject&)	= delete;
	GameObject(GameObject&&)			= default;
	GameObject& operator=(GameObject&&) = default;

	~GameObject() {}

	id_t getId() const { return id; }

  public:
	std::shared_ptr<Model> model;
	MaterialId materialId = 0;

	glm::vec3 color {1.0f};
	TransformComponent transform {};
	std::unique_ptr<PointLightComponent> pointLight				= nullptr;
	std::unique_ptr<DirectionalLightComponent> directionalLight = nullptr;

  private:
	id_t id;
};

} // namespace MVE