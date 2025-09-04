#pragma once

#include <raylib.h>

namespace Engine
{
    class Renderer
    {
    public:
        bool Initialize();
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void Begin3D(Camera3D camera);
        void End3D();

        void ResizeFrameTexture(int width, int height);
        void RenderImage(const Color* buffer, int width, int height);

        Camera3D* GetCamera();
        const Texture2D& GetFrameTexture() const;

    private:
        Camera3D m_Camera = { 0 };
        Texture2D m_FrameTexture = { 0 };
    };
}