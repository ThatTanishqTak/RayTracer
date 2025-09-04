#include "Core/Layer.h"

namespace Engine
{
    Layer::Layer(const std::string& layerName) : m_DebugName(layerName)
    {
        // Constructor simply stores the provided name for later use.
    }

    const std::string& Layer::GetLayerName() const
    {
        // Return the human readable identifier for this layer.
        return m_DebugName;
    }
}