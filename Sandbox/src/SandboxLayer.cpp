#include "SandboxLayer.h"

#include "Renderer/Material.h"
#include "Tracer/RayTracer.h"
#include "Tracer/Ray.h"
#include "Tracer/Sphere.h"

#include <raymath.h>
#include <rlImGui.h>
#include <imgui.h>

#include <vector>
#include <memory>
#include <random>

namespace
{
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
}

void SandboxLayer::OnAttach()
{
    float l_AspectRatio = 16.0f / 9.0f;
    int l_ImageWidth = 400;
    int l_ImageHeight = static_cast<int>(l_ImageWidth / l_AspectRatio);
    int l_SamplesPerPixel = 10;
    int l_MaxDepth = 10;

    Vector3 l_Origin{ 0.0f, 0.0f, 0.0f };
    float l_ViewportHeight = 2.0f;
    float l_ViewportWidth = l_ViewportHeight * l_AspectRatio;
    float l_FocalLength = 1.0f;

    Vector3 l_Horizontal{ l_ViewportWidth, 0.0f, 0.0f };
    Vector3 l_Vertical{ 0.0f, l_ViewportHeight, 0.0f };
    Vector3 l_LowerLeftCorner = Vector3Subtract(Vector3Subtract(Vector3Subtract(l_Origin, Vector3Scale(l_Horizontal, 0.5f)), Vector3Scale(l_Vertical, 0.5f)), 
        Vector3{ 0.0f, 0.0f, l_FocalLength });

    auto a_MaterialGround = std::make_shared<Engine::Lambertian>(Vector3{ 0.8f, 0.8f, 0.0f });
    auto a_MaterialCenter = std::make_shared<Engine::Lambertian>(Vector3{ 0.1f, 0.2f, 0.5f });
    auto a_MaterialLeft = std::make_shared<Engine::Dielectric>(1.5f);
    auto a_MaterialRight = std::make_shared<Engine::Metal>(Vector3{ 0.8f, 0.6f, 0.2f }, 0.0f);

    std::vector<Engine::Sphere> l_World;
    l_World.emplace_back(Vector3{ 0.0f, -100.5f, -1.0f }, 100.0f, a_MaterialGround);
    l_World.emplace_back(Vector3{ 0.0f, 0.0f, -1.0f }, 0.5f, a_MaterialCenter);
    l_World.emplace_back(Vector3{ -1.0f, 0.0f, -1.0f }, 0.5f, a_MaterialLeft);
    l_World.emplace_back(Vector3{ 1.0f, 0.0f, -1.0f }, 0.5f, a_MaterialRight);

    std::vector<Color> l_FrameBuffer(l_ImageWidth * l_ImageHeight);

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

            l_Color = Vector3Scale(l_Color, 1.0f / static_cast<float>(l_SamplesPerPixel));
            l_Color.x = sqrtf(l_Color.x);
            l_Color.y = sqrtf(l_Color.y);
            l_Color.z = sqrtf(l_Color.z);
            int l_Index = it_Y * l_ImageWidth + it_X;
            l_FrameBuffer[l_Index] = { static_cast<unsigned char>(255.999f * l_Color.x), static_cast<unsigned char>(255.999f * l_Color.y), 
                static_cast<unsigned char>(255.999f * l_Color.z), 255 };
        }
    }

    m_Renderer->RenderImage(l_FrameBuffer.data(), l_ImageWidth, l_ImageHeight);
}

void SandboxLayer::OnDetach()
{

}

void SandboxLayer::OnUpdate(float deltaTime)
{

}

void SandboxLayer::OnImGuiRender()
{
    rlImGuiBegin();

    rlImGuiEnd();
}

void SandboxLayer::OnSceneRender()
{
    DrawTexture(m_Renderer->GetFrameTexture(), 0, 0, WHITE);
}