#include "Application.h"

#include "Timestep.h"

namespace MVE
{
Application* Application::s_Instance = nullptr;

void Application::Run()
{
	// init
	for (auto& m : modules) { m->OnAttach(); }

	Timestep dt {0.0f};
	float lastFrameTime = glfwGetTime();
	float currentTime;

	// loop
	while (!window.ShouldClose()) {
		currentTime	  = glfwGetTime();
		dt			  = currentTime - lastFrameTime;
		lastFrameTime = currentTime;
		dt			  = glm::min((float)dt, MAX_FRAME_TIME);

		window.OnUpdate();
		for (auto& m : modules) { m->OnUpdate(dt); }
	}
}
} // namespace MVE