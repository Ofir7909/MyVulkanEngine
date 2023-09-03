#include "Render3DModule.h"

namespace MVE
{

void Render3DModule::OnAttach()
{
	materialSystem = std::make_unique<MaterialSystem>(device);
	LoadGameObjects();

	globalPool = DescriptorPool::Builder(device)
					 .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
					 .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
					 .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
					 .Build();

	globalUboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
		globalUboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
													   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
													   device.properties.limits.minUniformBufferOffsetAlignment);
		globalUboBuffers[i]->Map();
	}

	globalSetLayout = DescriptorSetLayout::Builder(device)
						  .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
						  .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
						  .Build();

	auto skyboxImageInfo = skyboxCubemap->ImageInfo();
	globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {
		auto bufferInfo = globalUboBuffers[i]->DescriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalPool)
			.WriteBuffer(0, &bufferInfo)
			.WriteImage(1, &skyboxImageInfo)
			.Build(globalDescriptorSets[i]);
	}

	pbrRenderSystem	 = std::make_unique<PbrRenderSystem>(device, renderer.GetSwapChainRenderPass(),
														 globalSetLayout->GetDescriptorSetLayout(), *materialSystem);
	pointLightSystem = std::make_unique<PointLightSystem>(device, renderer.GetSwapChainRenderPass(),
														  globalSetLayout->GetDescriptorSetLayout());
	skyboxSystem	 = std::make_unique<SkyboxSystem>(device, renderer.GetSwapChainRenderPass(),
												  globalSetLayout->GetDescriptorSetLayout());
}

void Render3DModule::OnDetach()
{
}

void Render3DModule::OnUpdate(Timestep dt)
{
	float aspect = renderer.GetAspectRatio();
	camera.SetPerspectiveProjection(glm::radians(65.0f), aspect, 0.1f, 100.0f);

	{ // TEMP: Camera controller
		static GameObject cameraTransform = GameObject::Create();
		glm::vec3 move {0.0f};
		move.x = Input::GetKey(KeyCode::D) - Input::GetKey(KeyCode::A);
		move.z = Input::GetKey(KeyCode::W) - Input::GetKey(KeyCode::S);
		move.y = Input::GetKey(KeyCode::E) - Input::GetKey(KeyCode::Q);
		if (move != glm::vec3 {0.0f}) {
			cameraTransform.transform.translation += move * (float)dt;
			camera.SetViewTarget(cameraTransform.transform.translation, {0.0f, 0.0f, 0.0f});
		}
	}

	auto commandBuffer = renderer.BeginFrame();
	if (commandBuffer) {
		int frameIndex = renderer.GetFrameIndex();
		FrameInfo frameInfo {frameIndex, dt, commandBuffer, camera, globalDescriptorSets[frameIndex]};

		GlobalUbo ubo {};
		ubo.view		= camera.GetView();
		ubo.projection	= camera.GetProjection();
		ubo.inverseView = camera.GetInverseView();
		pointLightSystem->Update(frameInfo, gameObjects, ubo);

		globalUboBuffers[frameIndex]->WriteToBuffer(&ubo);
		globalUboBuffers[frameIndex]->Flush();

		renderer.BeginSwapChainRenderPass(commandBuffer);

		pbrRenderSystem->RenderGameObjects(frameInfo, gameObjects, *materialSystem);
		skyboxSystem->Render(frameInfo, *skyboxCubemap);
		pointLightSystem->Render(frameInfo, gameObjects);

		renderer.EndSwapChainRenderPass(commandBuffer);
	}
	renderer.EndFrame();
}

void Render3DModule::LoadGameObjects()
{
	auto vaseSceneSetup = [&]() {
		std::shared_ptr model = Model::CreateModelFromFile(device, RES_DIR "models/smooth_vase.obj");

		// Materials
		auto redMatId			= materialSystem->CreateMaterial();
		auto& redMat			= materialSystem->Get(redMatId);
		redMat.params.albedo	= glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
		redMat.params.roughness = 0.85f;
		redMat.params.metallic	= 0.0f;

		auto goldMatId			 = materialSystem->CreateMaterial();
		auto& goldMat			 = materialSystem->Get(goldMatId);
		goldMat.params.albedo	 = glm::vec4(0.944f, 0.776f, 0.373f, 1.0f);
		goldMat.params.roughness = 0.3f;
		goldMat.params.metallic	 = 1.0f;

		auto floorMatId			 = materialSystem->CreateMaterial();
		auto& floorMat			 = materialSystem->Get(floorMatId);
		floorMat.params.uvScale	 = glm::vec2 {5.0f};
		floorMat.textures.albedo = Texture::Builder(device)
									   .addLayer(FileTextureSource(RES_DIR "textures/floor/slate_floor_diff_2k.jpg"))
									   .build();
		floorMat.textures.arm = Texture::Builder(device)
									.addLayer(FileTextureSource(RES_DIR "textures/floor/slate_floor_arm_2k.jpg"))
									.format(VK_FORMAT_R8G8B8A8_UNORM)
									.build();
		floorMat.textures.normal = Texture::Builder(device)
									   .addLayer(FileTextureSource(RES_DIR "textures/floor/slate_floor_nor_gl_2k.jpg"))
									   .format(VK_FORMAT_R8G8B8A8_UNORM)
									   .build();

		// Objects
		auto object					 = GameObject::Create();
		object.model				 = model;
		object.transform.translation = {0.5f, 0.0f, 0.0f};
		object.transform.scale		 = glm::vec3 {3.0f};
		object.materialId			 = redMatId;

		gameObjects.emplace(object.getId(), std::move(object));

		object						 = GameObject::Create();
		object.model				 = model;
		object.transform.translation = {-0.5f, 0.0f, 0.0f};
		object.transform.scale		 = glm::vec3 {1.0f, 0.3f, 0.5f} * 3.0f;
		object.materialId			 = goldMatId;

		gameObjects.emplace(object.getId(), std::move(object));

		std::shared_ptr floor = Model::CreateModelFromFile(device, RES_DIR "models/quad.obj");

		object						 = GameObject::Create();
		object.model				 = floor;
		object.transform.translation = {0.0f, 0.0f, 0.0f};
		object.transform.scale		 = glm::vec3 {5.0f};
		object.materialId			 = floorMatId;
		gameObjects.emplace(object.getId(), std::move(object));

		// Lights

		object						 = GameObject::CreatePointLight(0.5f, 0.1f, {0.5f, 0.1f, 0.1f});
		object.transform.translation = {-1.0f, -1.0f, -1.0f};
		gameObjects.emplace(object.getId(), std::move(object));

		object						 = GameObject::CreatePointLight(1.0f, 0.2f, {0.1f, 0.1f, 0.5f});
		object.transform.translation = {1.0f, -1.0f, -1.0f};
		gameObjects.emplace(object.getId(), std::move(object));

		object						 = GameObject::CreatePointLight(0.7f, 0.15f, {1.0f, 1.0f, 1.0f});
		object.transform.translation = {0.0f, -0.5f, 0.0f};
		gameObjects.emplace(object.getId(), std::move(object));

		object					  = GameObject::CreateDirectionalLight(1.5f, {1.0f, 1.0f, 1.0f});
		object.transform.rotation = {0.7f, 0.0f, -1.5f};
		gameObjects.emplace(object.getId(), std::move(object));
	};

	auto sphereSceneSetup = [&]() {
		std::shared_ptr sphere = Model::CreateModelFromFile(device, RES_DIR "models/sphere.obj");
		int n				   = 6;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				auto matId			 = materialSystem->CreateMaterial();
				auto& mat			 = materialSystem->Get(matId);
				mat.params.albedo	 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				mat.params.roughness = 1.0 / (n - 1) * i;
				mat.params.metallic	 = 1.0 / (n - 1) * j;

				auto object					 = GameObject::Create();
				object.model				 = sphere;
				object.transform.translation = glm::vec3 {i - (n - 1) / 2.0f, j - (n - 1) / 2.0f, 0.0f};
				object.transform.scale		 = glm::vec3 {0.15f};
				object.materialId			 = matId;
				gameObjects.emplace(object.getId(), std::move(object));
			}
		}

		// Lights
		auto light				 = GameObject::CreateDirectionalLight(1.5f, {1.0f, 1.0f, 1.0f});
		light.transform.rotation = {0.7f, 0.0f, -1.0f};
		gameObjects.emplace(light.getId(), std::move(light));
	};

	// skyboxCubemap = std::make_shared<Cubemap>(device, RES_DIR "cubemaps/skies/", "png");
	skyboxCubemap = std::make_shared<Cubemap>(device);
	skyboxCubemap->CreateFromHdri(RES_DIR "hdri/bush_restaurant_2k.hdr", 1024);

	// vaseSceneSetup();
	sphereSceneSetup();
}
} // namespace MVE