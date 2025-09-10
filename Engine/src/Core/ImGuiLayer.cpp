#include "Core/ImGuiLayer.h"

#include <raylib.h>
#include <rlImGui.h>

namespace Engine
{
    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& l_IO = ImGui::GetIO();
        l_IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // TODO: expose docking toggle
        // TODO: add multi-viewport support

        ImGui::StyleColorsDark(); // TODO: allow runtime style/theme selection

        ImGui_ImplRaylib_Init();
    }

    void ImGuiLayer::OnDetach()
    {
        // Release any textures registered through ImGui's platform IO
        // before tearing down the backend.

        // Shutdown the Raylib backend and destroy the ImGui context.
        ImGui_ImplRaylib_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::BeginFrame()
    {
        rlImGuiBegin();
    }

    void ImGuiLayer::EndFrame()
    {
        rlImGuiEnd();
    }
}