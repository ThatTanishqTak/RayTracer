#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"
#include "Tracer/RayTracerRenderer.h"
#include "Scene/Scene.h"

#include <raylib.h>

class SandboxLayer : public Engine::Layer
{
public:
    /**\brief Construct the layer and store a reference to the real-time renderer.*/
    SandboxLayer(Engine::Renderer* displayRenderer);
    virtual ~SandboxLayer() = default;

    /**\brief Called once when the layer is attached to the application.*/
    virtual void OnAttach() override;
    /**\brief Called when the layer is detached from the application.*/
    virtual void OnDetach() override;

    /**\brief Update logic executed every frame.*/
    virtual void OnUpdate(float deltaTime) override;
    /**\brief Draw ImGui elements for this layer.*/
    virtual void OnImGuiRender() override;
    /**\brief Render 2D and 3D scene elements.*/
    virtual void OnSceneRender() override;

private:
    Engine::Renderer* m_DisplayRenderer = nullptr;   /// Renderer used for real-time preview.
    Engine::RayTracerRenderer m_Renderer{};          /// Offline ray tracer.
    Engine::Scene m_Scene{};                        /// Scene data to render.
    bool m_RequestRender{ false };                  /// Flag set when user presses Render.
    float m_RenderTime = 0.0f;                      /// Time taken to render the scene in ms.
    float m_RenderProgress = 0.0f;                  /// Progress of current render in [0,1].
};