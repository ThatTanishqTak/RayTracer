#include "Window/Window.h"
#include "Utilities/Utilities.h"

#include <raylib.h>

namespace Engine
{
	bool Window::Initialize(int width, int height, const char* title)
	{
		RAY_CORE_INFO("Initializing window");

		SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
		InitWindow(width, height, title);
		
		SetExitKey(KEY_NULL);
		MaximizeWindow();

		RAY_CORE_INFO("Window initialized");

		return true;
	}

	void Window::Shutdown()
	{
		CloseWindow();
	}

	bool Window::ShouldClose() const
	{
		return WindowShouldClose();
	}
}