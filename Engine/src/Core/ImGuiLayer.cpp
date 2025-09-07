#include "Core/ImGuiLayer.h"

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
        ImGui_ImplRaylib_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::BeginFrame()
    {
        ImGui_ImplRaylib_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
    }
}