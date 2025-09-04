#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"

#include <raylib.h>

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
    Engine::Renderer* m_Renderer = nullptr;
    float m_RenderTime = 0.0f; // Added to store render time
};