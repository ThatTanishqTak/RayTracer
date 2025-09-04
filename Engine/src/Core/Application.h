#pragma once

#include "Window/Window.h"
#include "Core/LayerStack.h"
#include "Renderer/Renderer.h"

#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

#include <sstream>
#include <memory>

namespace Engine
{
	struct ApplicationSpecifications
	{
		int Width = 1080;
		int Height = 720;
		const char* Title = "Raylib-Application";
	};

	class Application
	{
	public:
		Application(const ApplicationSpecifications& specifications);
		~Application();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Renderer* GetRenderer() const;

		virtual void Run()
		{
			// Main loop
			while (!m_Window->ShouldClose())
			{
				float l_DeltaTime = GetFrameTime();

				// Render
				m_Renderer->BeginFrame();
				{
					// Scene phase
					for (Layer* it_Layer : m_LayerStack)
					{
						it_Layer->OnSceneRender();
					}

					// ImGui phase
					rlImGuiBegin();
					for (Layer* it_Layer : m_LayerStack)
					{
						it_Layer->OnImGuiRender();
					}
					rlImGuiEnd();
				}
				m_Renderer->EndFrame();

				// Update phase
				for (Layer* it_Layer : m_LayerStack)
				{
					it_Layer->OnUpdate(l_DeltaTime);
				}
			}
		}

	private:
		std::unique_ptr<Window> m_Window;
		std::shared_ptr<Renderer> m_Renderer;

		LayerStack m_LayerStack;
	};
}