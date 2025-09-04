#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"

#include <raylib.h>
#include <vector>

class SandboxLayer : public Engine::Layer
{
public:
    SandboxLayer(Engine::Renderer* renderer);
    virtual ~SandboxLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(float deltaTime) override;
    virtual void OnImGuiRender() override;
    virtual void OnSceneRender() override;

private:
    void RenderScene(int width, int height);

    Engine::Renderer* m_Renderer = nullptr;
    float m_RenderTime = 0.0f; // Added to store render time
    int m_ImageWidth = 0;
    int m_ImageHeight = 0;
    std::vector<Color> m_FrameBuffer;
};