#include "Core/Application.h"
#include "Utilities/Utilities.h"

namespace Engine
{
    Application::Application(const ApplicationSpecifications& specifications)
    {
        // Initialize core subsystems in a defined order.
        RAY_CORE_INFO("-------INITALIZING APPLICATION-------");

        // Disable raylib internal logging as the engine provides its own logging.
        SetTraceLogLevel(LOG_NONE);

        // Create and initialize the window using the user supplied specifications.
        m_Window = std::make_unique<Window>();
        if (!m_Window->Initialize(specifications.Width, specifications.Height, specifications.Title))
        {
            RAY_CORE_ERROR("Failed to initialize window");
        }

        // Construct the renderer and set it up for rendering.
        m_Renderer = std::make_shared<Renderer>();
        if (!m_Renderer->Initialize())
        {
            RAY_CORE_ERROR("Failed to initialize renderer");
        }

        RAY_CORE_INFO("-------APPLICATION INITIALIZED-------");
    }

    Application::~Application()
    {
        // Clean up resources in reverse order of creation.
        RAY_CORE_INFO("-------SHUTING DOWN APPLICATION-------");

        m_Renderer->Shutdown();
        m_Window->Shutdown();

        RAY_CORE_INFO("-------APPLICATION SHUTDOWN COMPLETE-------");
    }

    void Application::PushLayer(Layer* layer)
    {
        // Insert the layer and allow it to set up any state it requires.
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        // Overlays sit on top of normal layers and are also attached immediately.
        m_LayerStack.PopOverlay(overlay);
        overlay->OnAttach();
    }

    Renderer* Application::GetRenderer() const
    {
        // Provide raw access to the renderer for external modules.
        return m_Renderer.get();
    }
}