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

        bool l_Result = true;
        rlImGuiSetup(l_Result);

        return l_Result;
    }

    void Renderer::Shutdown()
    {
        if (m_FrameTexture.id != 0)
        {
            UnloadTexture(m_FrameTexture);
        }

        rlImGuiShutdown();
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

    void Renderer::RenderImage(const Color* buffer, int width, int height)
    {
        if (m_FrameTexture.id == 0 || m_FrameTexture.width != width || m_FrameTexture.height != height)
        {
            if (m_FrameTexture.id != 0)
            {
                UnloadTexture(m_FrameTexture);
            }

            Image l_Image{};
            l_Image.data = const_cast<Color*>(buffer);
            l_Image.width = width;
            l_Image.height = height;
            l_Image.mipmaps = 1;
            l_Image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            m_FrameTexture = LoadTextureFromImage(l_Image);
        }

        else
        {
            UpdateTexture(m_FrameTexture, buffer);
        }
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