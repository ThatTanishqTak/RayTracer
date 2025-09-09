#include "Renderer/Renderer.h"
#include "Utilities/Logging.h"

#include <raylib.h>
#include <rlImGui.h>

#include <algorithm>
#include <format>

namespace Engine
{
    bool Renderer::Initialize(int width, int height)
    {
        RAY_CORE_TRACE("Initializing the renderer");

        bool l_Result = true; // Track overall initialization success

        // Initialize ImGui for raylib and store the result.
        rlImGuiSetup(true);
        bool l_RlImGuiInitialized = true;
        if (!l_RlImGuiInitialized)
        {
            // Log failure and abort initialization if ImGui setup fails.
            RAY_CORE_ERROR("Failed to initialize rlImGui");
            l_Result = false;

            return l_Result;
        }

        // Allocate a render texture matching the current window size.
        if (l_Result)
        {
            l_Result = ResizeFrameTexture(width, height);
        }

        // Future improvement: provide detailed error codes instead of a simple boolean.

        RAY_CORE_TRACE("Renderer initialized");

        return l_Result;
    }

    void Renderer::Shutdown()
    {
        RAY_CORE_TRACE("Shuting down renderer");

        // Release frame texture if it exists.
        if (m_RenderTexture.id != 0)
        {
            UnloadRenderTexture(m_RenderTexture);
        }

        // Tear down ImGui integration.
        //rlImGuiShutdown();

        RAY_CORE_TRACE("Renderer shutdown complete");
    }

    void Renderer::BeginFrame()
    {
        // Prepare the screen for drawing.
        BeginDrawing();
        ClearBackground(BLACK);
    }

    void Renderer::EndFrame()
    {
        // Present the rendered frame to the display.
        EndDrawing();
    }

    void Renderer::Begin3D(Camera3D camera)
    {
        // Switch to 3D rendering mode.
        BeginMode3D(camera);
    }

    void Renderer::End3D()
    {
        // Return to 2D rendering mode.
        EndMode3D();
    }

    void Renderer::UpdateCamera(float deltaTime)
    {
        // Delegate input handling to the camera controller.
        // Centralizing camera logic simplifies future extensions such as custom bindings or editor modes.
        m_CameraController.Update(deltaTime);
    }

    bool Renderer::ResizeFrameTexture(int width, int height)
    {
        // Avoid reallocating the texture if dimensions are unchanged.
        if (width == m_FrameWidth && height == m_FrameHeight)
        {
            return true;
        }

        // Release previous render texture if it exists.
        if (m_RenderTexture.id != 0)
        {
            UnloadRenderTexture(m_RenderTexture);
            m_RenderTexture = { 0 };
        }

        // Create a new render texture; this enables potential direct drawing without CPU-GPU copy.
        RenderTexture2D l_NewTexture = LoadRenderTexture(width, height);
        if (l_NewTexture.id == 0)
        {
            // Preformat message to leverage C++20 formatting and avoid macro-specific format strings
            std::string l_Message = std::format("Failed to allocate render texture {} x {}", width, height);
            RAY_CORE_ERROR(l_Message);

            m_FrameWidth = 0;
            m_FrameHeight = 0;
            m_CachedPixels.clear();

            return false;
        }

        m_RenderTexture = l_NewTexture;
        m_FrameWidth = width;
        m_FrameHeight = height;

        // Ensure cached pixel buffer matches new dimensions.
        m_CachedPixels.assign(static_cast<size_t>(width) * static_cast<size_t>(height), Color{ 0, 0, 0, 255 });
        
        // Future improvement: handle different pixel formats and color spaces.
        return true;
    }

    void Renderer::RenderImage(const Color* buffer, int width, int height)
    {
        // Ensure texture matches incoming buffer dimensions.
        if (!ResizeFrameTexture(width, height))
        {
            return; // Abort rendering if texture allocation failed
        }

        // Determine the region of pixels that changed since the last upload.
        bool l_Changed = false;
        int l_MinX = width;
        int l_MinY = height;
        int l_MaxX = 0;
        int l_MaxY = 0;

        for (int it_Y = 0; it_Y < height; ++it_Y)
        {
            for (int it_X = 0; it_X < width; ++it_X)
            {
                int l_Index = it_Y * width + it_X;
                const Color& l_NewColor = buffer[l_Index];
                Color& l_OldColor = m_CachedPixels[l_Index];

                if (l_NewColor.r != l_OldColor.r || l_NewColor.g != l_OldColor.g || l_NewColor.b != l_OldColor.b || l_NewColor.a != l_OldColor.a)
                {
                    l_OldColor = l_NewColor;
                    l_Changed = true;
                    if (it_X < l_MinX)
                    {
                        l_MinX = it_X;
                    }

                    if (it_Y < l_MinY)
                    {
                        l_MinY = it_Y;
                    }

                    if (it_X > l_MaxX)
                    {
                        l_MaxX = it_X;
                    }

                    if (it_Y > l_MaxY)
                    {
                        l_MaxY = it_Y;
                    }
                }
            }
        }

        // Nothing to update.
        if (!l_Changed)
        {
            return;
        }

        int l_UpdateWidth = l_MaxX - l_MinX + 1;
        int l_UpdateHeight = l_MaxY - l_MinY + 1;

        if (l_UpdateWidth == width && l_UpdateHeight == height)
        {
            // Entire texture changed.
            UpdateTexture(m_RenderTexture.texture, m_CachedPixels.data());

            return;
        }

        // Copy changed region into a persistent sub-buffer to minimize allocations.
        size_t l_SubBufferSize = static_cast<size_t>(l_UpdateWidth) * static_cast<size_t>(l_UpdateHeight);
        if (m_SubBuffer.size() != l_SubBufferSize)
        {
            // Future improvement: reserve capacity in larger chunks or use tiling for massive images.
            m_SubBuffer.resize(l_SubBufferSize);
        }

        for (int it_Y = 0; it_Y < l_UpdateHeight; ++it_Y)
        {
            Color* l_Dst = &m_SubBuffer[static_cast<size_t>(it_Y) * static_cast<size_t>(l_UpdateWidth)];
            const Color* l_Src = &m_CachedPixels[(l_MinY + it_Y) * width + l_MinX];
            std::copy(l_Src, l_Src + l_UpdateWidth, l_Dst);
        }

        Rectangle l_Rect
        {
            static_cast<float>(l_MinX),
            static_cast<float>(l_MinY),
            static_cast<float>(l_UpdateWidth),
            static_cast<float>(l_UpdateHeight)
        };

        UpdateTextureRec(m_RenderTexture.texture, l_Rect, m_SubBuffer.data());

        // Future improvement: implement tiled texture updates to further reduce CPU-GPU transfers.
    }

    const Camera& Renderer::GetCamera() const
    {
        return m_CameraController.GetCamera();
    }

    const Texture2D& Renderer::GetFrameTexture() const
    {
        return m_RenderTexture.texture;
    }

    int Renderer::GetTileSize() const
    {
        // Current tile side length in pixels.
        // Future improvement: expose runtime configuration or adaptive sizing.
        return m_TileSize;
    }

    void Renderer::SetRenderMode(RenderMode a_Mode)
    {
        // Switch between rasterization and ray tracing pipelines.
        m_RenderMode = a_Mode;
        // Future improvement: reconfigure shaders or dispatch GPU compute tasks for hybrid rendering.
    }

    Renderer::RenderMode Renderer::GetRenderMode() const
    {
        // Report current rendering technique.
        return m_RenderMode;
    }
}