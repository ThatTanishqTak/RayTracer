#pragma once

#include "Core/Layer.h"

#include <vector>

namespace Engine
{
    /**
     *\brief Container managing an ordered collection of layers and overlays.
     */
    class LayerStack
    {
    public:
        LayerStack() = default;
        /**\brief Release all layers and call their detachment handlers.*/
        ~LayerStack();

        /**\brief Insert a layer below all overlays.*/
        void PushLayer(Layer* layer);
        /**\brief Append an overlay to the top of the stack.*/
        void PushOverlay(Layer* overlay);
        /**\brief Remove a previously inserted layer.*/
        void PopLayer(Layer* layer);
        /**\brief Remove an overlay from the stack.*/
        void PopOverlay(Layer* overlay);

        // Iteration helpers for traversing the internal layer array
        std::vector<Layer*>::iterator begin();
        std::vector<Layer*>::iterator end();
        std::vector<Layer*>::reverse_iterator rbegin();
        std::vector<Layer*>::reverse_iterator rend();

        std::vector<Layer*>::const_iterator begin() const;
        std::vector<Layer*>::const_iterator end() const;
        std::vector<Layer*>::const_reverse_iterator rbegin() const;
        std::vector<Layer*>::const_reverse_iterator rend() const;

    private:
        std::vector<Layer*> m_Layers; ///< Stored layer pointers.
        unsigned int m_LayerInsertIndex = 0; ///< Index separating layers and overlays.
    };
}