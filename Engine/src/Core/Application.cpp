#include "Core/Application.h"
#include "Utilities/Utilities.h"

namespace Engine
{
	Application::Application(const ApplicationSpecifications& specifications)
	{

		RAY_CORE_INFO("-------INITALIZING APPLICATION-------");

		SetTraceLogLevel(LOG_NONE);

		m_Window = std::make_unique<Window>();
		if (!m_Window->Initialize(specifications.Width, specifications.Height, specifications.Title))
		{
			RAY_CORE_ERROR("Failed to initialize window");
		}

		m_Renderer = std::make_shared<Renderer>();
		if (!m_Renderer->Initialize())
		{
			RAY_CORE_ERROR("Failed to initialize renderer");
		}

		RAY_CORE_INFO("-------APPLICATION INITIALIZED-------");
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