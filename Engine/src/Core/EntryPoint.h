#pragma once

#include "Core/Application.h"

namespace Engine
{
	Application* CreateApplication();
}

int main()
{
	auto a_App = Engine::CreateApplication();
	a_App->Run();

	delete a_App;

	return 0;
}