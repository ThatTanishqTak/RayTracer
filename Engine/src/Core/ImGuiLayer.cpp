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

        // Iterate over each texture, release associated GPU resources, and
        // remove it from the list to avoid double cleanup on subsequent
        // shutdowns.
        while (l_PlatformIO.Textures.Size > 0)
        {
            // Retrieve and remove the first texture entry. Removing the entry
            // up front ensures the list is cleared even if early continues
            // occur.
            ImTextureData* l_Texture = l_PlatformIO.Textures[0];
            l_PlatformIO.Textures.erase(l_PlatformIO.Textures.begin());

            // Safety check: skip null entries before accessing Status to avoid
            // potential null-pointer dereferences.
            if (l_Texture == nullptr)
            {
                continue;
            }

            if (l_Texture->Status != ImTextureStatus_Destroyed)
            {
                Texture2D* l_BackendData = reinterpret_cast<Texture2D*>(l_Texture->BackendUserData);

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

                l_Texture->BackendUserData = nullptr;
                l_Texture->Status = ImTextureStatus_Destroyed;
                l_Texture->SetTexID(ImTextureID_Invalid);
            }
        }

        // Clear the texture list to avoid accidental reuse of freed textures.
        l_PlatformIO.Textures.clear();
    }
}