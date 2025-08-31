#include "Core/Application.h"

namespace Engine
{
	Application::Application()
	{
		m_Renderer = std::make_unique<Renderer>();
		if (!m_Renderer->Initialize())
		{
			std::cout << "Failed to initialize renderer" << std::endl;
		}
	}

	Application::~Application()
	{
		m_Renderer->Shutdown();
	}
}