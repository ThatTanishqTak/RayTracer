#include "Core/LayerStack.h"

namespace Engine
{
    LayerStack::~LayerStack()
    {
        // Ensure each layer detaches and is deleted in order.
        for (Layer* it_Layer : m_Layers)
        {
            it_Layer->OnDetach();

            delete it_Layer;
        }
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        // Insert layer before overlays and increase insert index.
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex++;
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        // Overlays are simply appended to the container.
        m_Layers.emplace_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        // Locate the layer within the managed range and remove it.
        auto a_SelectedLayer = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
        if (a_SelectedLayer != m_Layers.begin() + m_LayerInsertIndex)
        {
            layer->OnDetach();
            m_Layers.erase(a_SelectedLayer);

            m_LayerInsertIndex--;
        }
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        // Overlays are searched in the overlay range and removed if found.
        auto a_SelectedOverlay = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
        if (a_SelectedOverlay != m_Layers.end())
        {
            overlay->OnDetach();
            m_Layers.erase(a_SelectedOverlay);
        }
    }

    std::vector<Layer*>::iterator LayerStack::begin()
    {
        // Begin iterator for range-based loops.
        return m_Layers.begin();
    }

    std::vector<Layer*>::iterator LayerStack::end()
    {
        // End iterator for range-based loops.
        return m_Layers.end();
    }

    std::vector<Layer*>::reverse_iterator LayerStack::rbegin()
    {
        return m_Layers.rbegin();
    }

    std::vector<Layer*>::reverse_iterator LayerStack::rend()
    {
        return m_Layers.rend();
    }

    std::vector<Layer*>::const_iterator LayerStack::begin() const
    {
        return m_Layers.begin();
    }

    std::vector<Layer*>::const_iterator LayerStack::end() const
    {
        return m_Layers.end();
    }

    std::vector<Layer*>::const_reverse_iterator LayerStack::rbegin() const
    {
        return m_Layers.rbegin();
    }

    std::vector<Layer*>::const_reverse_iterator LayerStack::rend() const
    {
        return m_Layers.rend();
    }
}