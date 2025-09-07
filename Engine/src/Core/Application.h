#pragma once

#include "Window/Window.h"
#include "Core/LayerStack.h"
#include "Renderer/Renderer.h"
#include "Core/ImGuiLayer.h"

#include <raylib.h>

#include <sstream>
#include <memory>

namespace Engine
{
    /**
     *\brief Contains basic information required to configure the application window.
     */
    struct ApplicationSpecifications
    {
        int Width = 1080;
        int Height = 720;
        const char* Title = "Raylib-Application";
    };

    /**
     *\brief Central engine class managing window creation, rendering and layer updates.
     */
    class Application
    {
    public:
        /**
         *\brief Construct the application using the provided specifications.
         *\param specifications Desired size and title for the main window.
         */
        Application(const ApplicationSpecifications& specifications);

        /**\brief Release owned resources and shutdown the application.*/
        ~Application();

        /**
         *\brief Insert a new layer into the stack and notify it of attachment.
         *\param layer Pointer to the layer that will be managed by the application.
         */
        void PushLayer(Layer* layer);

        /**
         *\brief Add an overlay on top of the regular layers and attach it.
         *\param layer Pointer to the overlay layer.
         */
        void PushOverlay(Layer* layer);

        /**\brief Retrieve the renderer associated with the application.*/
        Renderer* GetRenderer() const;

        /**
         *\brief Main execution loop. Handles rendering and updating of all layers
         *       until the window requests closure.
         */
        virtual void Run()
        {
            // Main loop
            while (!m_Window->ShouldClose())
            {
                // Detect window size changes and update the renderer to match.
                if (IsWindowResized())
                {
                    int l_Width = GetScreenWidth();
                    int l_Height = GetScreenHeight();
                    
                    m_Renderer->ResizeFrameTexture(l_Width, l_Height); // Prevent image clipping
                    // Future improvement: adjust camera aspect ratio on resize.
                }

                float l_DeltaTime = GetFrameTime();
                m_Renderer->UpdateCamera(l_DeltaTime); // Handle camera navigation

                // Render
                m_Renderer->BeginFrame();
                {
                    // Scene phase
                    for (Layer* it_Layer : m_LayerStack)
                    {
                        it_Layer->OnSceneRender();
                    }

                    // ImGui phase
                    m_ImGuiLayer->BeginFrame();
                    for (Layer* it_Layer : m_LayerStack)
                    {
                        it_Layer->OnImGuiRender();
                    }
                    m_ImGuiLayer->EndFrame();
                }
                m_Renderer->EndFrame();

                // Update phase
                for (Layer* it_Layer : m_LayerStack)
                {
                    it_Layer->OnUpdate(l_DeltaTime);
                }
            }
        }

    private:
        std::unique_ptr<Window> m_Window; /// Handle to the OS window instance.
        std::shared_ptr<Renderer> m_Renderer; /// Renderer responsible for drawing.
        std::unique_ptr<ImGuiLayer> m_ImGuiLayer; /// Overlay responsible for ImGui rendering.

        LayerStack m_LayerStack; /// Ordered collection of active layers.
    };
}