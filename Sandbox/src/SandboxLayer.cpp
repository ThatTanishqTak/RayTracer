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
	BeginMode3D(*m_Renderer->GetCamera());

	DrawGrid(10000, 10.0f);
	DrawCube({ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, RED);

	EndMode3D();
}