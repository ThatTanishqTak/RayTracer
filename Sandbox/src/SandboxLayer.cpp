#include "SandboxLayer.h"

#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

SandboxLayer::SandboxLayer(Engine::Renderer* renderer) : Engine::Layer("SandboxLayer"), m_Renderer(renderer)
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
	UpdateCamera(m_Renderer->GetCamera(), CAMERA_FREE);
}

void SandboxLayer::OnImGuiRender()
{
	rlImGuiBegin();

	ImGui::Text("TEST");

	rlImGuiEnd();
}

void SandboxLayer::OnSceneRender()
{
	DrawTexture(m_Renderer->GetFrameTexture(), 0, 0, WHITE);
}