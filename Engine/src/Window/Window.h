#pragma once

namespace Engine
{
    /**
     *\brief Lightweight wrapper around the underlying raylib window functions.
     */
    class Window
    {
    public:
        /**\brief Create the application window with the desired size and title.*/
        bool Initialize(int width, int height, const char* title);
        /**\brief Destroy the window and related resources.*/
        void Shutdown();

        /**\brief Check whether the user has requested the window to close.*/
        bool ShouldClose() const;
    };
}