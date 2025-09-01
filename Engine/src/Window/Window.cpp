#include "Window/Window.h"

#include <raylib.h>

namespace Engine
{
	bool Window::Initialize(int width, int height, const char* title)
	{
		SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

		InitWindow(width, height, title);
		MaximizeWindow();

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