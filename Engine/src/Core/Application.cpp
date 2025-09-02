#include "Core/Application.h"

#include <iostream>

namespace Engine
{
	Application::Application(const ApplicationSpecifications& specifications)
	{
		m_Window = std::make_unique<Window>();
		if (!m_Window->Initialize(specifications.Width, specifications.Height, specifications.Title))
		{
			std::cout << "Failed to initialize window" << std::endl;
		}

		m_Renderer = std::make_shared<Renderer>();
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

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PopOverlay(overlay);
		overlay->OnAttach();
	}

	Renderer* Application::GetRenderer() const
	{
		return m_Renderer.get();
	}
}