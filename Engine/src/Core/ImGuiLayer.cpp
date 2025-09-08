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
        CleanupTextures();

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

    void ImGuiLayer::CleanupTextures()
    {
        // Retrieve texture list from ImGui's platform interface.
        ImGuiPlatformIO& l_PlatformIO = ImGui::GetPlatformIO();

        // Iterate over each texture and release associated GPU resources.
        for (ImTextureData* it_Texture : l_PlatformIO.Textures)
        {
            if (it_Texture->Status != ImTextureStatus_Destroyed)
            {
                Texture2D* l_BackendData = reinterpret_cast<Texture2D*>(it_Texture->BackendUserData);

                // Ensure texture is valid before unloading.
                if (l_BackendData && IsTextureValid(*l_BackendData))
                {
                    UnloadTexture(*l_BackendData);
                }

                // Free memory allocated for the backend texture data.
                if (l_BackendData)
                {
                    MemFree(l_BackendData);
                }

                it_Texture->BackendUserData = nullptr;
                it_Texture->Status = ImTextureStatus_Destroyed;
                it_Texture->SetTexID(ImTextureID_Invalid);
            }
        }
    }
}