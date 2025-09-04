#include "SandboxLayer.h"

#include "Renderer/Material.h"
#include "Tracer/RayTracer.h"
#include "Tracer/Ray.h"
#include "Tracer/Sphere.h"
#include "Utilities/Utilities.h"

#include <raymath.h>
#include <rlImGui.h>
#include <imgui.h>

#include <iostream>
#include <memory>
#include <random>
#include <chrono>
#include <vector>

namespace
{
    // Generate a random float in the range [0,1).
    float RandomFloat()
    {
        static std::uniform_real_distribution<float> s_Distribution(0.0f, 1.0f);
        static std::mt19937 s_Generator(std::random_device{}());
        float l_Value = s_Distribution(s_Generator);

        return l_Value;
    }
}

SandboxLayer::SandboxLayer(Engine::Renderer* renderer) : Engine::Layer("SandboxLayer"), m_Renderer(renderer)
{
    // Constructor simply stores the renderer pointer.
}

void SandboxLayer::OnAttach()
{
    // Cache initial window dimensions and render the scene once.
    m_ImageWidth = GetScreenWidth();
    m_ImageHeight = GetScreenHeight();

    RenderScene(m_ImageWidth, m_ImageHeight);
}

void SandboxLayer::OnDetach()
{
    // No special cleanup required for this layer.
}

void SandboxLayer::OnUpdate(float deltaTime)
{
    int l_CurrentWidth = GetScreenWidth();
    int l_CurrentHeight = GetScreenHeight();

    // Re-render if the window size changes.
    if (l_CurrentWidth != m_ImageWidth || l_CurrentHeight != m_ImageHeight)
    {
        m_ImageWidth = l_CurrentWidth;
        m_ImageHeight = l_CurrentHeight;

        RenderScene(m_ImageWidth, m_ImageHeight);
    }
}

void SandboxLayer::OnImGuiRender()
{
    // Create the "Render Stats" window and ensure it stays on top
    ImGui::Begin("Render Stats", nullptr);
    ImGui::Text("Render Time: %.4f ms", m_RenderTime);
    ImGui::End();

    ImGui::SetWindowFocus("Render Stats");
}

void SandboxLayer::OnSceneRender()
{
    // Blit the ray traced frame buffer to the screen.
    DrawTexture(m_Renderer->GetFrameTexture(), 0, 0, WHITE);
}

void SandboxLayer::RenderScene(int width, int height)
{
    int l_ImageWidth = width;
    int l_ImageHeight = height;
    float l_AspectRatio = static_cast<float>(l_ImageWidth) / static_cast<float>(l_ImageHeight);
    int l_SamplesPerPixel = 10;
    int l_MaxDepth = 10;

    // Camera setup.
    Vector3 l_Origin{ 0.0f, 0.0f, 0.0f };
    float l_ViewportHeight = 2.0f;
    float l_ViewportWidth = l_ViewportHeight * l_AspectRatio;
    float l_FocalLength = 1.0f;

    Vector3 l_Horizontal{ l_ViewportWidth, 0.0f, 0.0f };
    Vector3 l_Vertical{ 0.0f, l_ViewportHeight, 0.0f };
    Vector3 l_LowerLeftCorner = Vector3Subtract(Vector3Subtract(Vector3Subtract(l_Origin, Vector3Scale(l_Horizontal, 0.5f)),
        Vector3Scale(l_Vertical, 0.5f)), { 0.0f, 0.0f, l_FocalLength });

    // Scene setup with a few spheres of different materials.
    auto a_MaterialGround = std::make_shared<Engine::Lambertian>(Vector3{ 0.8f, 0.8f, 0.0f });
    auto a_MaterialCenter = std::make_shared<Engine::Lambertian>(Vector3{ 0.1f, 0.2f, 0.5f });
    auto a_MaterialLeft = std::make_shared<Engine::Dielectric>(1.5f);
    auto a_MaterialRight = std::make_shared<Engine::Metal>(Vector3{ 0.8f, 0.6f, 0.2f }, 0.0f);

    std::vector<Engine::Sphere> l_World;
    l_World.emplace_back(Vector3{ 0.0f, -100.5f, -1.0f }, 100.0f, a_MaterialGround);
    l_World.emplace_back(Vector3{ 0.0f, 0.0f, -1.0f }, 0.5f, a_MaterialCenter);
    l_World.emplace_back(Vector3{ -1.0f, 0.0f, -1.0f }, 0.5f, a_MaterialLeft);
    l_World.emplace_back(Vector3{ 1.0f, 0.0f, -1.0f }, 0.5f, a_MaterialRight);

    m_FrameBuffer.resize(l_ImageWidth * l_ImageHeight);

    auto a_Start = std::chrono::high_resolution_clock::now();

    // Ray trace each pixel with multiple samples for anti-aliasing.
    for (int it_Y = l_ImageHeight - 1; it_Y >= 0; --it_Y)
    {
        for (int it_X = 0; it_X < l_ImageWidth; ++it_X)
        {
            Vector3 l_Color{ 0.0f, 0.0f, 0.0f };
            for (int it_Sample = 0; it_Sample < l_SamplesPerPixel; ++it_Sample)
            {
                float l_U = (static_cast<float>(it_X) + RandomFloat()) / (l_ImageWidth - 1);
                float l_V = (static_cast<float>(it_Y) + RandomFloat()) / (l_ImageHeight - 1);

                Vector3 l_Direction = Vector3Add(Vector3Add(l_LowerLeftCorner, Vector3Scale(l_Horizontal, l_U)), Vector3Scale(l_Vertical, l_V));
                l_Direction = Vector3Subtract(l_Direction, l_Origin);
                Engine::Ray l_Ray(l_Origin, l_Direction);
                Vector3 l_SampleColor = Engine::RayColor(l_Ray, l_World, l_MaxDepth);

                l_Color = Vector3Add(l_Color, l_SampleColor);
            }

            // Average all samples and apply gamma correction.
            l_Color = Vector3Scale(l_Color, 1.0f / static_cast<float>(l_SamplesPerPixel));
            l_Color.x = sqrtf(l_Color.x);
            l_Color.y = sqrtf(l_Color.y);
            l_Color.z = sqrtf(l_Color.z);

            int l_Index = it_Y * l_ImageWidth + it_X;
            m_FrameBuffer[l_Index] = { static_cast<unsigned char>(255.999f * l_Color.x), static_cast<unsigned char>(255.999f * l_Color.y),
                static_cast<unsigned char>(255.999f * l_Color.z), 255 };
        }
    }

    auto a_End = std::chrono::high_resolution_clock::now();
    m_RenderTime = std::chrono::duration<float, std::milli>(a_End - a_Start).count();

    // Upload the finished image to the GPU for display.
    m_Renderer->ResizeFrameTexture(l_ImageWidth, l_ImageHeight);
    m_Renderer->RenderImage(m_FrameBuffer.data(), l_ImageWidth, l_ImageHeight);
}