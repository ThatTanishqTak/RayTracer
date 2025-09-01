#pragma once

#include <string>

namespace Engine
{
	class Layer
	{
	public:
		Layer(const std::string& layerName = "Default");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent() {}

		const std::string& GetLayerName() const;

	protected:
		std::string m_DebugName;
	};
}