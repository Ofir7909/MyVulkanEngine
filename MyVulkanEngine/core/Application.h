#pragma once

#include "Input.h"
#include "Window.h"
#include "moduels/Module.h"

// #include<vector>

namespace MVE
{
class Application
{
  private:
	Application() { Log::Init(); }

  public:
	static Application* Create()
	{
		MVE_ASSERT(s_Instance == nullptr, "Application already exists. There can only be one application");
		s_Instance = new Application();
		return s_Instance;
	}
	static Application* Get() { return s_Instance; }

  public:
	~Application()
	{
		for (auto& m : modules) {
			m->OnDetach();
			delete m;
		}
	};

	template<typename T>
	Application* AddModule()
	{
		modules.push_back(new T());
		return this;
	}

	void Run();

	Window& GetWindow() { return window; }

  private:
	static Application* s_Instance;

  private:
	static constexpr float MAX_FRAME_TIME = 1 / 30.0f;
	std::vector<Module*> modules;
	Window window;
}; // namespace MVE
} // namespace MVE