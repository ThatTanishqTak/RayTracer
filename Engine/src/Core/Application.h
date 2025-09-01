#pragma once

#include "Window/Window.h"
#include "Renderer/Renderer.h"

#include <raylib.h>

#include <sstream>
#include <memory>

namespace Engine
{
	struct ApplicationSpecifications
	{
		int Width = 1920;
		int Height = 1080;
		const char* Title = "Raylib-Application";
	};

	class Application
	{
	public:
		Application(const ApplicationSpecifications& specifications);
		~Application();

		virtual void Run()
		{
			while (!m_Window->ShouldClose())
			{
				m_Renderer->BeginFrame();

				DrawText(TextFormat("FPS: %d", GetFPS()), 0, 0, 24, RED);

				m_Renderer->EndFrame();
			}
		}

	private:
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer> m_Renderer;
	};
}