#include "Core/Layer.h"

namespace Engine
{
	Layer::Layer(const std::string& layerName) : m_DebugName(layerName)
	{

	}

	const std::string& Layer::GetLayerName() const
	{
		return m_DebugName;
	}
}