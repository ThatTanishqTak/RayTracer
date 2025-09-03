#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"
#include "Tracer/Sphere.h"

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
    void ApplySettings();

    Engine::Renderer* m_Renderer = nullptr;
    float m_CameraFov = 45.0f;
    Vector3 m_ObjectPosition{ 0.0f, 0.0f, 0.0f };
    Color m_MaterialColor{ 255, 255, 255, 255 };
    Engine::Sphere m_Sphere;

};