#include "Renderer/Renderer.h"

#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

namespace Engine
{
    bool Renderer::Initialize()
    {
        m_Camera.position = { 0.0f, 10.0f, 10.0f };
        m_Camera.target = { 0.0f, 0.0f, 0.0f };
        m_Camera.up = { 0.0f, 1.0f, 0.0f };
        m_Camera.fovy = 45.0f;
        m_Camera.projection = CAMERA_PERSPECTIVE;

        ImGui::CreateContext();
        ImGuiIO& l_IO = ImGui::GetIO(); (void)l_IO;
        ImGui::StyleColorsDark();
        rlImGuiSetup(true);

        bool l_Result = true;

        return l_Result;
    }

    void Renderer::Shutdown()
    {
        if (m_FrameTexture.id != 0)
        {
            UnloadTexture(m_FrameTexture);
        }

        rlImGuiShutdown();
        ImGui::DestroyContext();
    }

    void Renderer::BeginFrame()
    {
        BeginDrawing();
        ClearBackground(BLACK);
    }

    void Renderer::EndFrame()
    {
        EndDrawing();
    }

    void Renderer::Begin3D(Camera3D camera)
    {
        BeginMode3D(camera);
    }

    void Renderer::End3D()
    {
        EndMode3D();
    }

    void Renderer::ResizeFrameTexture(int width, int height)
    {
        if (m_FrameTexture.id != 0)
        {
            UnloadTexture(m_FrameTexture);
        }

        Image l_Image = GenImageColor(width, height, BLACK);
        m_FrameTexture = LoadTextureFromImage(l_Image);
        UnloadImage(l_Image);
    }

    void Renderer::RenderImage(const Color* buffer, int width, int height)
    {
        UpdateTexture(m_FrameTexture, buffer);
    }

    Camera3D* Renderer::GetCamera()
    {
        return &m_Camera;
    }

    const Texture2D& Renderer::GetFrameTexture() const
    {
        return m_FrameTexture;
    }
}