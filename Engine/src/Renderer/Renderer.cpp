#include "Renderer/Renderer.h"
#include "Utilities/Utilities.h"

#include <raylib.h>
#include <rlImGui.h>
#include <raymath.h>

#include <algorithm>
#include <type_traits>

namespace Engine
{
    bool Renderer::Initialize()
    {
        RAY_CORE_INFO("Initializing the renderer");

        bool l_Result = true; // Track overall initialization success

        // Set up a default perspective camera.
        m_Camera.position = { 0.0f, 10.0f, 10.0f };
        m_Camera.target = { 0.0f, 0.0f, 0.0f };
        m_Camera.up = { 0.0f, 1.0f, 0.0f };
        m_Camera.fovy = 45.0f;
        m_Camera.projection = CAMERA_PERSPECTIVE;

        // Initialize ImGui for raylib. Capture potential return value if provided.
        using l_RlImGuiReturn = decltype(rlImGuiSetup(true));
        if constexpr (std::is_same_v<l_RlImGuiReturn, void>)
        {
            rlImGuiSetup(true);
        }

        else
        {
            return false;
        }

        // Allocate a minimal render texture to verify GPU resources.
        if (l_Result)
        {
            l_Result = ResizeFrameTexture(1, 1);
        }

        // Future improvement: provide detailed error codes instead of a simple boolean.

        RAY_CORE_INFO("Renderer initialized");

        return l_Result;
    }

    void Renderer::Shutdown()
    {
        RAY_CORE_INFO("Shuting down renderer");

        // Release frame texture if it exists.
        if (m_RenderTexture.id != 0)
        {
            UnloadRenderTexture(m_RenderTexture);
        }

        // Tear down ImGui integration.
        rlImGuiShutdown();

        RAY_CORE_INFO("Renderer shutdown complete");
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
        // Unity-style fly camera: hold right mouse button to look around and use WASDQE for movement.
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            DisableCursor();

            Vector2 l_MouseDelta = GetMouseDelta();
            float l_RotateSpeed = 0.1f;

            // Rotate the forward vector based on mouse movement.
            Vector3 l_Forward = Vector3Subtract(m_Camera.target, m_Camera.position);
            Matrix l_Rotation = MatrixRotateXYZ(Vector3{ -l_MouseDelta.y * DEG2RAD * l_RotateSpeed, -l_MouseDelta.x * DEG2RAD * l_RotateSpeed, 0.0f });
            l_Forward = Vector3Transform(l_Forward, l_Rotation);

            Vector3 l_Right = Vector3Normalize(Vector3CrossProduct(l_Forward, m_Camera.up));
            Vector3 l_Up = Vector3Normalize(Vector3CrossProduct(l_Right, l_Forward));

            float l_MoveSpeed = 5.0f;
            if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                l_MoveSpeed *= 2.0f;
            }

            // Move the camera based on keyboard input.
            if (IsKeyDown(KEY_W)) { m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Forward, l_MoveSpeed * deltaTime)); }
            if (IsKeyDown(KEY_S)) { m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Forward, l_MoveSpeed * deltaTime)); }
            if (IsKeyDown(KEY_A)) { m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Right, l_MoveSpeed * deltaTime)); }
            if (IsKeyDown(KEY_D)) { m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Right, l_MoveSpeed * deltaTime)); }
            if (IsKeyDown(KEY_Q)) { m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Up, l_MoveSpeed * deltaTime)); }
            if (IsKeyDown(KEY_E)) { m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Up, l_MoveSpeed * deltaTime)); }

            m_Camera.target = Vector3Add(m_Camera.position, l_Forward);
        }

        else
        {
            EnableCursor();
        }

        // Future improvement: expose movement speeds and input mapping to the user.
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
            RAY_CORE_ERROR("Failed to allocate render texture %d x %d", width, height);

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
                    if (it_X < l_MinX) { l_MinX = it_X; }
                    if (it_Y < l_MinY) { l_MinY = it_Y; }
                    if (it_X > l_MaxX) { l_MaxX = it_X; }
                    if (it_Y > l_MaxY) { l_MaxY = it_Y; }
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

        // Copy changed region to a contiguous temporary buffer.
        std::vector<Color> l_SubBuffer(static_cast<size_t>(l_UpdateWidth) * static_cast<size_t>(l_UpdateHeight));
        for (int it_Y = 0; it_Y < l_UpdateHeight; ++it_Y)
        {
            Color* l_Dst = &l_SubBuffer[static_cast<size_t>(it_Y) * static_cast<size_t>(l_UpdateWidth)];
            const Color* l_Src = &m_CachedPixels[(l_MinY + it_Y) * width + l_MinX];
            std::copy(l_Src, l_Src + l_UpdateWidth, l_Dst);
        }

        Rectangle l_Rect{ static_cast<float>(l_MinX), static_cast<float>(l_MinY), static_cast<float>(l_UpdateWidth), static_cast<float>(l_UpdateHeight) };

        UpdateTextureRec(m_RenderTexture.texture, l_Rect, l_SubBuffer.data());
    }

    Camera3D* Renderer::GetCamera()
    {
        return &m_Camera;
    }

    const Texture2D& Renderer::GetFrameTexture() const
    {
        return m_RenderTexture.texture;
    }
}