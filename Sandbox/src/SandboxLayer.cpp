#include "SandboxLayer.h"

#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

SandboxLayer::SandboxLayer() : Engine::Layer("SandboxLayer")
{

}

void SandboxLayer::OnAttach()
{

}

void SandboxLayer::OnDetach()
{

}

void SandboxLayer::OnUpdate(float deltaTime)
{
	if (IsKeyDown(KEY_TAB))
	{
		UpdateCameraPro(m_Render.GetCamera(), { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 1.0f);
	}
}

void SandboxLayer::OnImGuiRender()
{
	ImGui::Text("WHAT");
}

void SandboxLayer::OnSceneRender()
{
	DrawGrid(100, 10.0f);
	DrawCube({ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, RED);
}