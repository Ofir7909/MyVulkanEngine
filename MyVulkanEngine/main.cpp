#include "Core/Application.h"
#include "moduels/render3d/Render3DModule.h"

int main()
{
	using namespace MVE;

	auto app = Application::Create()->AddModule<Render3DModule>();

	app->Run();

	delete app;
}