#include "Render3DModule.h"

namespace MVE
{

void Render3DModule::OnAttach()
{
	GenerateBrdfLut();

	materialSystem = std::make_unique<MaterialSystem>(device);
	LoadGameObjects();

	globalPool = DescriptorPool::Builder(device)
					 .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
					 .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
					 .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 3)
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
						  .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
						  .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
						  .Build();

	auto skyboxImageInfo	 = skyboxCubemap->ImageInfo();
	auto irradianceImageInfo = skyboxCubemap->IrradianceImageInfo();
	auto brdfLutImageInfo	 = brdfLut->ImageInfo();
	globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {
		auto bufferInfo = globalUboBuffers[i]->DescriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalPool)
			.WriteBuffer(0, &bufferInfo)
			.WriteImage(1, &skyboxImageInfo)
			.WriteImage(2, &irradianceImageInfo)
			.WriteImage(3, &brdfLutImageInfo)
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

	{ // Temp: New Camera Controller
		constexpr float mouseSensitivity = 3.0f;
		constexpr float pitchMinMax		 = glm::half_pi<float>() - 0.01f;
		static bool firstRun			 = true;
		static glm::vec3 cameraPosition {0.0, 0.0, 1.0};
		static float yaw   = 0.0f;
		static float pitch = 0.0f;
		static glm::vec2 mousePosPrev;
		if (firstRun) {
			glm::quat rotationQuat {glm::vec3 {pitch, yaw, 0.0f}};
			camera.SetViewDirection(cameraPosition, glm::rotate(rotationQuat, glm::vec3 {0.0, 0.0, -1.0}));
			firstRun = false;
		}

		auto mousePosNew = Input::GetMousePosition();
		auto mouseDelta	 = mousePosNew - mousePosPrev;
		mousePosPrev	 = mousePosNew;

		// fly Cam Navigation(Right mouse button)
		if (Input::GetMouseButton(1)) {
			// Mouse look
			yaw -= mouseDelta.x * mouseSensitivity * dt;
			pitch -= mouseDelta.y * mouseSensitivity * dt;
			MVE_INFO(pitch);
			pitch = glm::clamp(pitch, -pitchMinMax, pitchMinMax);
			glm::quat rotationQuat {glm::vec3 {pitch, yaw, 0.0f}};

			// Movement
			glm::vec2 input		= Input::GetVector(KeyCode::A, KeyCode::D, KeyCode::W, KeyCode::S);
			float verticalInput = Input::GetAxis(KeyCode::Q, KeyCode::E);
			glm::vec3 dir		= glm::rotate(rotationQuat, glm::vec3 {input.x, 0.0, input.y});
			dir.y += verticalInput;
			cameraPosition += dir * (float)dt;

			auto cameraForward = glm::rotate(rotationQuat, glm::vec3 {0.0, 0.0, -1.0});
			camera.SetViewDirection(cameraPosition, cameraForward);
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
		object.transform.rotation = glm::quat({0.7f, 0.0f, -1.5f});
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
				mat.params.roughness = float(i) / (n - 1);
				mat.params.metallic	 = float(j) / (n - 1);

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
		light.transform.rotation = glm::quat({0.7f, 0.0f, -1.0f});
		gameObjects.emplace(light.getId(), std::move(light));
	};

	auto cerberusSceneSetup = [&]() {
		std::shared_ptr model = Model::CreateModelFromFile(device, RES_DIR "models/Cerberus/Cerberus_LP.FBX");

		auto materialId		= materialSystem->CreateMaterial();
		auto& mat			= materialSystem->Get(materialId);
		mat.textures.albedo = Texture::Builder(device)
								  .addLayer(FileTextureSource(RES_DIR "models/Cerberus/Textures/Cerberus_A.tga"))
								  .build();
		mat.textures.arm = Texture::Builder(device)
							   .addLayer(FileTextureSource(RES_DIR "models/Cerberus/Textures/Cerberus_ORM.tga"))
							   .format(VK_FORMAT_R8G8B8A8_UNORM)
							   .build();
		mat.textures.normal = Texture::Builder(device)
								  .addLayer(FileTextureSource(RES_DIR "models/Cerberus/Textures/Cerberus_N.tga"))
								  .format(VK_FORMAT_R8G8B8A8_UNORM)
								  .build();

		auto object					 = GameObject::Create();
		object.model				 = model;
		object.transform.translation = {0.0f, 0.0f, 0.0f};
		object.transform.scale		 = glm::vec3 {0.01f};
		object.transform.rotation	 = glm::quat(glm::radians(glm::vec3 {-90.0f, 90.0f, 0.0f}));
		object.materialId			 = materialId;
		gameObjects.emplace(object.getId(), std::move(object));
	};

	skyboxCubemap = std::make_shared<Cubemap>(device);
	// skyboxCubemap->CreateFromHdri(RES_DIR "hdri/bush_restaurant_2k.hdr", 512);
	skyboxCubemap->CreateFromHdri(RES_DIR "hdri/clarens_midday_2k.hdr", 512);

	// vaseSceneSetup();
	// sphereSceneSetup();
	cerberusSceneSetup();
}

void Render3DModule::GenerateBrdfLut(uint32_t resolution)
{
	brdfLut = Texture::Builder(device)
				  .format(VK_FORMAT_R16G16_SFLOAT)
				  .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				  .addUsageFlag(VK_IMAGE_USAGE_STORAGE_BIT)
				  .layout(VK_IMAGE_LAYOUT_GENERAL)
				  .addLayer(SolidTextureSource(glm::vec4 {}, resolution, resolution))
				  .build();

	auto descriptorPool =
		DescriptorPool::Builder(device).SetMaxSets(1).AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1).Build();

	auto setLayout = DescriptorSetLayout::Builder(device)
						 .AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
						 .Build();

	auto brdfLutInfo = brdfLut->ImageInfo();

	VkDescriptorSet set;
	DescriptorWriter(*setLayout, *descriptorPool).WriteImage(0, &brdfLutInfo).Build(set);

	// create pipeline
	std::unique_ptr<ComputePipeline> pipeline;
	VkPipelineLayout pipelineLayout;

	std::vector<VkDescriptorSetLayout> descripotorSetLayouts {setLayout->GetDescriptorSetLayout()};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges	  = VK_NULL_HANDLE;

	vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	pipeline = std::make_unique<ComputePipeline>(device, SHADER_BINARY_DIR "brdfLutGenerator.comp.spv", pipelineLayout);

	// render
	auto commandBuffer = device.BeginSingleTimeCommands();

	pipeline->Bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr);

	const glm::ivec3 shaderLocalSize {32, 32, 1};
	vkCmdDispatch(commandBuffer, brdfLut->width() / shaderLocalSize.x, brdfLut->width() / shaderLocalSize.y, 1);

	device.EndSingleTimeCommands(commandBuffer);
	vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
}
} // namespace MVE