#pragma once

#include <string>

namespace Engine
{
    /**
     *\brief Base interface for any execution layer such as game logic or UI overlays.
     */
    class Layer
    {
    public:
        /**
         *\brief Create a new layer instance with an optional debug name.
         *\param layerName Human readable name used for logging and debugging.
         */
        Layer(const std::string& layerName = "Default");
        virtual ~Layer() = default;

        /**\brief Called when the layer is added to the application.*/
        virtual void OnAttach() {}
        /**\brief Called when the layer is removed from the application.*/
        virtual void OnDetach() {}
        /**\brief Per-frame update using the elapsed time since the last frame.*/
        virtual void OnUpdate(float deltaTime) {}
        /**\brief Invoked during the scene rendering phase.*/
        virtual void OnSceneRender() {}
        /**\brief Invoked during the ImGui rendering phase.*/
        virtual void OnImGuiRender() {}
        /**\brief Receive and process events sent to the layer.*/
        virtual void OnEvent() {}

        /**\brief Retrieve the debug name assigned to this layer.*/
        const std::string& GetLayerName() const;

    protected:
        std::string m_DebugName; ///< User provided identifier for the layer.
    };
}