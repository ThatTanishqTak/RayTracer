#include "Core/Application.h"
#include "Utilities/Logging.h"

#include <stdexcept>

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
        const bool l_WindowInitialized = m_Window->Initialize(specifications.Width, specifications.Height, specifications.Title);
        if (!l_WindowInitialized)
        {
            RAY_CORE_ERROR("Failed to initialize window");

            // Ensure any partially created resources are released before aborting construction.
            m_Window->Shutdown();
            m_Window.reset();

            // TODO: Introduce a retry mechanism to attempt window creation again.
        }

        // Construct the renderer and set it up for rendering.
        m_Renderer = std::make_shared<Renderer>();
        // Initialize the renderer using the current window dimensions to avoid clipped output.
        const bool l_RendererInitialized = m_Renderer->Initialize(specifications.Width, specifications.Height);
        if (!l_RendererInitialized)
        {
            RAY_CORE_ERROR("Failed to initialize renderer");

            // Clean up previously initialized subsystems to avoid leaks.
            m_Renderer->Shutdown();
            m_Renderer.reset();

            m_Window->Shutdown();
            m_Window.reset();

            // TODO: Allow retrying renderer initialization without destroying the application object.
        }

        // Create and register the ImGui overlay layer.
        m_ImGuiLayer = std::make_unique<ImGuiLayer>();
        PushOverlay(m_ImGuiLayer.get());

        RAY_CORE_INFO("-------APPLICATION INITIALIZED-------");
    }

    Application::~Application()
    {
        // Clean up resources in reverse order of creation.
        RAY_CORE_INFO("-------SHUTING DOWN APPLICATION-------");

        // Remove overlay to avoid double deletion.
        m_LayerStack.PopOverlay(m_ImGuiLayer.get());

        // Delete the ImGui layer once.
        m_ImGuiLayer.reset();

        if (m_Renderer)
        {
            m_Renderer->Shutdown();
        }

        if (m_Window)
        {
            m_Window->Shutdown();
        }

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
        // Use PushOverlay to add overlays instead of removing them.
        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }

    Renderer* Application::GetRenderer() const
    {
        // Provide raw access to the renderer for external modules.
        return m_Renderer.get();
    }
}