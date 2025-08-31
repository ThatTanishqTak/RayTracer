#include "Renderer/Renderer.h"

#include <raylib.h>

#include <memory>

namespace Engine
{
	class Application
	{
	public:
		Application();
		~Application();

		virtual void Run()
		{
			InitWindow(m_Width, m_Height, m_Title);
			SetTargetFPS(60);

			while (!WindowShouldClose())
			{
				BeginDrawing();

				m_Renderer->DrawFrame();
			
				EndDrawing();
			}
		}

	private:
		int m_Width = 1920;
		int m_Height = 1080;
		const char* m_Title = "RayTracer";

		std::unique_ptr<Renderer> m_Renderer;
	};
}