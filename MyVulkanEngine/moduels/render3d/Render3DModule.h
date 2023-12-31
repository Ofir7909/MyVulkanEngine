#pragma once

#include "Descriptors.h"
#include "Device.h"
#include "MaterialSystem.h"
#include "Renderer.h"
#include "moduels/Module.h"
#include "renderSystems/PbrRenderSystem.h"
#include "renderSystems/PointLightSystem.h"
#include "renderSystems/SkyboxSystem.h"

#include "core/Application.h"
#include "core/GameObject.h"

namespace MVE
{

class Render3DModule : public Module
{
  public:
	Render3DModule()  = default;
	~Render3DModule() = default;

	Render3DModule(const Render3DModule&) = delete;
	void operator=(const Render3DModule&) = delete;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Timestep dt) override;

  private:
	void LoadGameObjects();
	void GenerateBrdfLut(uint32_t resolution = 512);

  private:
	Device device {Application::Get()->GetWindow()};
	Renderer renderer {device};

	std::unique_ptr<DescriptorPool> globalPool;
	std::unique_ptr<DescriptorSetLayout> globalSetLayout;
	std::vector<VkDescriptorSet> globalDescriptorSets;
	GameObject::Map gameObjects;
	std::shared_ptr<Texture> brdfLut;

	std::vector<std::unique_ptr<Buffer>> globalUboBuffers;
	std::unique_ptr<MaterialSystem> materialSystem;
	std::shared_ptr<Cubemap> skyboxCubemap;

	std::unique_ptr<PbrRenderSystem> pbrRenderSystem;
	std::unique_ptr<PointLightSystem> pointLightSystem;
	std::unique_ptr<SkyboxSystem> skyboxSystem;
	Camera camera {};
};
} // namespace MVE