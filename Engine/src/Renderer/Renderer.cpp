#include "Renderer/Renderer.h"

#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

namespace Engine
{
	bool Renderer::Initialize()
	{
		m_Camera.position = { 0.0f, 10.0f, 10.0f };
		m_Camera.target = { 0.0f, 0.0f, 0.0f };
		m_Camera.up = { 0.0f, 1.0f, 0.0f };
		m_Camera.fovy = 45.0f;
		m_Camera.projection = CAMERA_PERSPECTIVE;

		bool l_Result = true;
		rlImGuiSetup(l_Result);

		return l_Result;
	}

	void Renderer::Shutdown()
	{
		rlImGuiShutdown();
	}

	void Renderer::BeginFrame()
	{
		BeginDrawing();
		ClearBackground(BLACK);
	}
	
	void Renderer::EndFrame()
	{
		EndDrawing();
	}

	void Renderer::Begin3D(Camera3D camera)
	{
		BeginMode3D(camera);
	}

	void Renderer::End3D()
	{
		EndMode3D();
	}

	Camera3D* Renderer::GetCamera()
	{
		return &m_Camera;
	}
}