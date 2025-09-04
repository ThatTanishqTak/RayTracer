#pragma once

#include <raylib.h>

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

        /**\brief Resize or recreate the internal frame texture.*/
        void ResizeFrameTexture(int width, int height);
        /**\brief Upload a color buffer into the frame texture.*/
        void RenderImage(const Color* buffer, int width, int height);

        /**\brief Access the camera used for 3D rendering.*/
        Camera3D* GetCamera();
        /**\brief Retrieve the current frame texture.*/
        const Texture2D& GetFrameTexture() const;

    private:
        Camera3D m_Camera = { 0 };       ///< Default perspective camera.
        Texture2D m_FrameTexture = { 0 }; ///< Texture containing last rendered frame.
    };
}