#include "SandboxLayer.h"

#include "Renderer/Material.h"
#include "Tracer/RayTracer.h"
#include "Tracer/Ray.h"
#include "Tracer/Sphere.h"
#include "Tracer/BVHNode.h"
#include "Utilities/Utilities.h"

#include <raymath.h>
#include <rlImGui.h>
#include <imgui.h>

#include <iostream>
#include <memory>
#include <random>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <algorithm>

namespace
{
    // Thread-local generator and distribution for sampling.
    thread_local std::mt19937 s_Generator(std::random_device{}());
    thread_local std::uniform_real_distribution<float> s_Distribution(0.0f, 1.0f);

    // Generate a random float in the range [0,1) using the supplied generator.
    float RandomFloat(std::mt19937& generator)
    {
        float l_Value = s_Distribution(generator);

        return l_Value;
    }
}

SandboxLayer::SandboxLayer(Engine::Renderer* renderer) : Engine::Layer("SandboxLayer"), m_Renderer(renderer)
{
    // Constructor simply stores the renderer pointer.
}

void SandboxLayer::OnAttach()
{
    // Allocate a frame buffer large enough for the biggest monitor size and
    // cache the current window dimensions for rendering.
    m_MaxImageWidth = GetMonitorWidth(GetCurrentMonitor());
    m_MaxImageHeight = GetMonitorHeight(GetCurrentMonitor());
    m_FrameBuffer.resize(m_MaxImageWidth * m_MaxImageHeight);

    const Color l_ClearColor{ 0, 0, 0, 255 };
    std::fill(m_FrameBuffer.begin(), m_FrameBuffer.end(), l_ClearColor);

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
    ImGui::Text("FPS: %d", GetFrameTime());
    ImGui::End();

    ImGui::SetWindowFocus("Render Stats");
}

void SandboxLayer::OnSceneRender()
{
    // Blit the ray traced frame buffer to the screen.
    const Texture2D& l_Texture = m_Renderer->GetFrameTexture();
    Rectangle l_Source{ 0.0f, 0.0f, static_cast<float>(l_Texture.width), static_cast<float>(-l_Texture.height) };
    DrawTextureRec(l_Texture, l_Source, Vector2{ 0.0f, 0.0f }, WHITE);
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

    // Build a BVH from the scene spheres using a deterministic local engine.
    std::mt19937 l_BvhEngine{ 0 };
    Engine::BVHNode l_Bvh(l_World, 0, l_World.size(), l_BvhEngine);

    const Color l_ClearColor{ 0, 0, 0, 255 };
    std::fill(m_FrameBuffer.begin(), m_FrameBuffer.begin() + static_cast<size_t>(l_ImageWidth * l_ImageHeight), l_ClearColor);

    std::atomic<float> l_TotalRenderTime = 0.0f;
    unsigned int l_ThreadCount = std::thread::hardware_concurrency();
    if (l_ThreadCount == 0)
    {
        l_ThreadCount = 1;
    }
    int l_RowsPerThread = l_ImageHeight / static_cast<int>(l_ThreadCount);

    std::vector<std::jthread> l_ThreadPool;
    l_ThreadPool.reserve(l_ThreadCount);

    for (unsigned int it_Thread = 0; it_Thread < l_ThreadCount; ++it_Thread)
    {
        int l_StartY = static_cast<int>(it_Thread) * l_RowsPerThread;
        int l_EndY = (it_Thread == l_ThreadCount - 1) ? l_ImageHeight : l_StartY + l_RowsPerThread;

        l_ThreadPool.emplace_back([=, this, &l_TotalRenderTime, &l_Bvh]()
            {
                std::mt19937& l_Generator = s_Generator; // Reuse thread-local RNG
                auto a_ThreadStart = std::chrono::high_resolution_clock::now();

                for (int it_Y = l_EndY - 1; it_Y >= l_StartY; --it_Y)
                {
                    for (int it_X = 0; it_X < l_ImageWidth; ++it_X)
                    {
                        Vector3 l_Color{ 0.0f, 0.0f, 0.0f };
                        for (int it_Sample = 0; it_Sample < l_SamplesPerPixel; ++it_Sample)
                        {
                            float l_U = (static_cast<float>(it_X) + RandomFloat(l_Generator)) / (l_ImageWidth - 1);
                            float l_V = (static_cast<float>(it_Y) + RandomFloat(l_Generator)) / (l_ImageHeight - 1);

                            Vector3 l_Direction = Vector3Add(Vector3Add(l_LowerLeftCorner, Vector3Scale(l_Horizontal, l_U)), Vector3Scale(l_Vertical, l_V));
                            l_Direction = Vector3Subtract(l_Direction, l_Origin);
                            Engine::Ray l_Ray(l_Origin, l_Direction);
                            Vector3 l_SampleColor = Engine::RayColor(l_Ray, l_Bvh, l_MaxDepth);

                            l_Color = Vector3Add(l_Color, l_SampleColor);
                        }

                        l_Color = Vector3Scale(l_Color, 1.0f / static_cast<float>(l_SamplesPerPixel));
                        l_Color.x = sqrtf(l_Color.x);
                        l_Color.y = sqrtf(l_Color.y);
                        l_Color.z = sqrtf(l_Color.z);

                        int l_Index = it_Y * l_ImageWidth + it_X;
                        m_FrameBuffer[l_Index] = { static_cast<unsigned char>(255.999f * l_Color.x), static_cast<unsigned char>(255.999f * l_Color.y),
                            static_cast<unsigned char>(255.999f * l_Color.z), 255 };
                    }
                }

                auto a_ThreadEnd = std::chrono::high_resolution_clock::now();
                float l_Duration = std::chrono::duration<float, std::milli>(a_ThreadEnd - a_ThreadStart).count();
                l_TotalRenderTime.fetch_add(l_Duration, std::memory_order_relaxed);
            });
    }

    for (std::jthread& it_Thread : l_ThreadPool)
    {
        it_Thread.join();
    }

    m_RenderTime = l_TotalRenderTime.load();

    // Upload the finished image to the GPU for display.
    // Only upload the image if the frame texture could be resized successfully.
    if (m_Renderer->ResizeFrameTexture(l_ImageWidth, l_ImageHeight))
    {
        m_Renderer->RenderImage(m_FrameBuffer.data(), l_ImageWidth, l_ImageHeight);
    }
}