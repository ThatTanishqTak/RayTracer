#include "Core/Application.h"

namespace Engine
{
	Application::Application(const ApplicationSpecifications& specifications)
	{
		m_Window = std::make_unique<Window>();
		if (!m_Window->Initialize(specifications.Width, specifications.Height, specifications.Title))
		{
			std::cout << "Failed to initialize window" << std::endl;
		}

		m_Renderer = std::make_unique<Renderer>();
		if (!m_Renderer->Initialize())
		{
			std::cout << "Failed to initialize renderer" << std::endl;
		}
	}

	Application::~Application()
	{
		m_Renderer->Shutdown();
		m_Window->Shutdown();
	}
}