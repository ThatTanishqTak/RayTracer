#include "SandboxLayer.h"

#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

SandboxLayer::SandboxLayer(Engine::Renderer* renderer) : Engine::Layer("SandboxLayer"), m_Renderer(renderer)
{
    m_Sphere = Engine::Sphere(m_ObjectPosition, 1.0f);
}

void SandboxLayer::OnAttach()
{

}

void SandboxLayer::OnDetach()
{

}

void SandboxLayer::OnUpdate(float deltaTime)
{
    ApplySettings();
    UpdateCamera(m_Renderer->GetCamera(), CAMERA_FREE);
}

void SandboxLayer::OnImGuiRender()
{
    rlImGuiBegin();

    ImGui::SliderFloat("Camera FOV", &m_CameraFov, 1.0f, 120.0f);
    ImGui::SliderFloat3("Object Position", &m_ObjectPosition.x, -10.0f, 10.0f);

    float l_Color[3] = { m_MaterialColor.r / 255.0f, m_MaterialColor.g / 255.0f, m_MaterialColor.b / 255.0f };
    if (ImGui::ColorEdit3("Material Color", l_Color))
    {
        m_MaterialColor.r = static_cast<unsigned char>(l_Color[0] * 255.0f);
        m_MaterialColor.g = static_cast<unsigned char>(l_Color[1] * 255.0f);
        m_MaterialColor.b = static_cast<unsigned char>(l_Color[2] * 255.0f);
    }

    rlImGuiEnd();
}

void SandboxLayer::OnSceneRender()
{
    DrawTexture(m_Renderer->GetFrameTexture(), 0, 0, WHITE);
}

void SandboxLayer::ApplySettings()
{
    m_Renderer->GetCamera()->fovy = m_CameraFov;
    m_Sphere = Engine::Sphere(m_ObjectPosition, 1.0f);
}