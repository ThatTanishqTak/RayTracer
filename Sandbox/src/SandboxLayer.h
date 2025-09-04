#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"

#include <raylib.h>
#include <vector>

class SandboxLayer : public Engine::Layer
{
public:
    /**\brief Construct the layer and store a reference to the renderer.*/
    SandboxLayer(Engine::Renderer* renderer);
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
    /**\brief Perform the offline ray tracing and upload results to the renderer.*/
    void RenderScene(int width, int height);

    Engine::Renderer* m_Renderer = nullptr; /// Renderer used to display the result.
    float m_RenderTime = 0.0f;              /// Time taken to render the scene in ms.
    int m_ImageWidth = 0;                   /// Current window width used for rendering.
    int m_ImageHeight = 0;                  /// Current window height used for rendering.
    int m_MaxImageWidth = 0;                /// Maximum width allocated for the frame buffer.
    int m_MaxImageHeight = 0;               /// Maximum height allocated for the frame buffer.
    std::vector<Color> m_FrameBuffer;       /// Single allocation reused for all renders.
};