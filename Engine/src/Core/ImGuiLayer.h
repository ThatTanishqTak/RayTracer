#pragma once

#include "Core/Layer.h"

#include <imgui.h>
#include <imgui_impl_raylib.h>

namespace Engine
{
    /**
     *\brief Handles Dear ImGui integration and rendering.
     */
    class ImGuiLayer : public Layer
    {
    public:
        /**\brief Construct a new ImGui overlay layer.*/
        ImGuiLayer() = default;

        /**\brief Initialize ImGui context and the Raylib backend.*/
        void OnAttach() override;
        /**\brief Shutdown the Raylib backend and destroy context.*/
        void OnDetach() override;

        /**\brief Begin a new ImGui frame.*/
        void BeginFrame();
        /**\brief Finalize rendering for the current ImGui frame.*/
        void EndFrame();

    private:
        /**\brief Release all ImGui textures before shutdown.
         *
         * Iterates over textures registered with ImGui's platform
         * interface and frees their Raylib resources. This prevents
         * access violations when the ImGui context is destroyed.
         * Future improvement: expose texture management to a dedicated
         * renderer or resource manager.
         */
        void CleanupTextures();
    };
}