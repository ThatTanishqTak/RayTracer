#include "Window/Window.h"
#include "Utilities/Logging.h"

#include <raylib.h>

namespace Engine
{
    bool Window::Initialize(int width, int height, const char* title)
    {
        RAY_CORE_INFO("Initializing window");

        // Configure the window and create it via raylib.
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
        InitWindow(width, height, title);

        // Disable default exit key and maximize for better user experience.
        SetExitKey(KEY_NULL);
        MaximizeWindow();

        RAY_CORE_INFO("Window initialized");

        return true;
    }

    void Window::Shutdown()
    {
        RAY_CORE_INFO("Shuting down window");

        // Close the window and clean up internal raylib resources.
        CloseWindow();

        RAY_CORE_INFO("Window shutdown complete");
    }

    bool Window::ShouldClose() const
    {
        // Query raylib for the close flag.
        return WindowShouldClose();
    }
}