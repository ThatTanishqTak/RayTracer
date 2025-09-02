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
		rlImGuiBegin();
		BeginMode3D(*GetCamera());
		ClearBackground(BLACK);

		TraceLog(LOG_INFO, TextFormat("%d, %d, %d", m_Camera.position.x, m_Camera.position.y, m_Camera.position.z));
	}
	
	void Renderer::EndFrame()
	{
		rlImGuiEnd();
		EndMode3D();
		EndDrawing();
	}

	Camera3D* Renderer::GetCamera()
	{
		return &m_Camera;
	}
}