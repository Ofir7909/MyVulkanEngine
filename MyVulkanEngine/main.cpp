#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Core/Application.h"
#include "moduels/render3d/Render3DModule.h"

void test(int x)
{
	using namespace MVE;
	MVE_INFO("This is a function x={}", x);
}

struct S
{
	void Test()
	{
		using namespace MVE;
		MVE_INFO("Member Function");
	}

	void TestWithParam(int x)
	{
		using namespace MVE;
		MVE_INFO("Member Function with paramater, x={}", x);
	}
};

int main()
{
	using namespace MVE;

	auto app = Application::Create()->AddModule<Render3DModule>();

	{ // Event test - Remove later
		MVE_INFO("\n");
		S s {};
		S s2 {};
		Event<> e;
		e += new MemFuncEventCallback(&s, &S::Test);
		e += new MemFuncEventCallback(&s, &S::Test);
		e();

		Event<int> e2;
		e2 += new MemFuncEventCallback(&s, &S::TestWithParam);
		e2 += new MemFuncEventCallback(&s, &S::TestWithParam);
		e2 += new MemFuncEventCallback(&s2, &S::TestWithParam);
		e2 += new GlobalFuncEventCallback(&test);
		e2(42);
		MVE_INFO("\n");
	}

	app->Run();

	delete app;
}