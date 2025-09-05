#pragma once

#include "Renderer/CameraController.h"

#include <raylib.h>
#include <vector>

namespace Engine
{
    /**
     *\brief Handles all drawing operations and provides a simple frame buffer interface.
     */
    class Renderer
    {
    public:
        /**\brief Initialize renderer state and prepare for drawing.*/
        bool Initialize();
        /**\brief Release resources allocated by the renderer.*/
        void Shutdown();

        /**\brief Start a new frame for 2D/3D rendering.*/
        void BeginFrame();
        /**\brief Finalize the current frame.*/
        void EndFrame();

        /**\brief Enter 3D drawing mode using the provided camera.*/
        void Begin3D(Camera3D camera);
        /**\brief Leave 3D drawing mode.*/
        void End3D();

        /**\brief Resize or recreate the internal frame texture. Returns false on allocation failure.*/
        bool ResizeFrameTexture(int width, int height);
        /**\brief Upload a color buffer into the frame texture. Optionally only update changed regions.*/
        void RenderImage(const Color* buffer, int width, int height);

        /**\brief Access the camera used for 3D rendering.*/
        const Camera& GetCamera() const;
        /**\brief Update the internal camera based on user input.*/
        void UpdateCamera(float deltaTime);
        /**\brief Retrieve the current frame texture.*/
        const Texture2D& GetFrameTexture() const;

    private:
        CameraController m_CameraController{}; ///Centralized camera controller for input.
        RenderTexture2D m_RenderTexture = { 0 }; ///Render texture backing the frame buffer.
        int m_FrameWidth = 0; ///Cached width of the frame texture.
        int m_FrameHeight = 0; ///Cached height of the frame texture.
        std::vector<Color> m_CachedPixels = { }; ///Copy of last uploaded pixel data.
        std::vector<Color> m_SubBuffer = { }; ///Reusable buffer for sub-region uploads to avoid frequent allocations.
    };
}